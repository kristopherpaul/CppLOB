#pragma once

#include "level.hpp"
#include "order.hpp"
#include "type_list.hpp"
#include <type_traits>

namespace lob::compile_time {

// Helper for best_level
template <bool IsEmpty, class Levels>
struct BestLevelHelper;

template <class... Levels>
struct BestLevelHelper<true, TypeList<Levels...>> {
    using type = void;
};

template <class Level, class... Levels>
struct BestLevelHelper<false, TypeList<Level, Levels...>> {
    using type = Level;
};

// Helper for best_price
template <bool IsEmpty, class Levels>
struct BestPriceHelper;

template <class... Levels>
struct BestPriceHelper<true, TypeList<Levels...>> {
    static constexpr spec::Price value = 0;
};

template <class Level, class... Levels>
struct BestPriceHelper<false, TypeList<Level, Levels...>> {
    static constexpr spec::Price value = Level::price;
};

template <class Level>
struct LevelContentsValid;

template <spec::Price PriceV, class... Orders>
struct LevelContentsValid<PriceLevel<PriceV, Orders...>>
    : std::bool_constant<
          (sizeof...(Orders) > 0) && (ValidOrder<Orders> && ...)> {};

template <class... Levels>
struct SideBook {
    using levels = TypeList<Levels...>;
    static constexpr std::size_t level_count = sizeof...(Levels);

    static constexpr bool is_empty = (level_count == 0);

    static constexpr bool valid = []() consteval {
        if constexpr (sizeof...(Levels) == 0) {
            return true;
        } else {
            return (LevelContentsValid<Levels>::value && ...);
        }
    }();

    using best_level = typename BestLevelHelper<is_empty, levels>::type;
    static constexpr spec::Price best_price = BestPriceHelper<is_empty, levels>::value;
};

// Comparison for sorting - by default descending (for bids)
template <class Level1, class Level2, bool IsDescending = true>
struct LevelCompare;

template <class Level1, class Level2>
struct LevelCompare<Level1, Level2, true> {
    static constexpr bool value = (Level1::price > Level2::price);
};

template <class Level1, class Level2>
struct LevelCompare<Level1, Level2, false> {
    static constexpr bool value = (Level1::price < Level2::price);
};

// Simpler insertion using a linear approach with customizable comparison
template <class NewLevel, class List, bool IsDescending = true>
struct InsertLevelSorted;

template <class NewLevel, bool IsDescending>
struct InsertLevelSorted<NewLevel, TypeList<>, IsDescending> {
    using type = TypeList<NewLevel>;
};

template <class NewLevel, class First, class... Rest, bool IsDescending>
struct InsertLevelSorted<NewLevel, TypeList<First, Rest...>, IsDescending> {
    static constexpr bool new_is_better = LevelCompare<NewLevel, First, IsDescending>::value;
    using type = std::conditional_t<
        new_is_better,
        TypeList<NewLevel, First, Rest...>,
        PushFront_t<First, typename InsertLevelSorted<NewLevel, TypeList<Rest...>, IsDescending>::type>
    >;
};

template <class Level, class List, bool IsDescending = true>
struct InsertLevel;

template <spec::Price PriceV, class... Orders, class... ExistingLevels>
struct InsertLevel<PriceLevel<PriceV, Orders...>, TypeList<ExistingLevels...>, true> {
    using type = typename InsertLevelSorted<PriceLevel<PriceV, Orders...>, TypeList<ExistingLevels...>, true>::type;
};

template <spec::Price PriceV, class... Orders, class... ExistingLevels>
struct InsertLevel<PriceLevel<PriceV, Orders...>, TypeList<ExistingLevels...>, false> {
    using type = typename InsertLevelSorted<PriceLevel<PriceV, Orders...>, TypeList<ExistingLevels...>, false>::type;
};

template <spec::Price PriceV, class... Orders>
struct InsertLevel<PriceLevel<PriceV, Orders...>, TypeList<>> {
    using type = TypeList<PriceLevel<PriceV, Orders...>>;
};

template <bool IsDescending, class Book, class NewOrder>
requires ValidOrder<NewOrder>
struct AddOrderToSide;

template <bool IsDescending, class... Levels, class NewOrder>
requires ValidOrder<NewOrder>
struct AddOrderToSide<IsDescending, SideBook<Levels...>, NewOrder> {
private:
    using NewLevel = PriceLevel<NewOrder::price, NewOrder>;

    template <class Level>
    struct SamePriceLevel {
        static constexpr bool value = (Level::price == NewOrder::price);
    };

    static constexpr std::size_t found_index = find_if_v<TypeList<Levels...>, SamePriceLevel>;
    static constexpr bool found = (found_index != static_cast<std::size_t>(-1));

    template <bool Found, std::size_t Index>
    struct AddImpl;

    template <std::size_t Index>
    struct AddImpl<true, Index> {
        using ExistingLevel = At_t<Index, TypeList<Levels...>>;
        using UpdatedLevel = LevelPushBack_t<ExistingLevel, NewOrder>;

        template <std::size_t I, class... Ls>
        struct ReplaceLevel;

        template <class L0, class... Ls>
        struct ReplaceLevel<0, L0, Ls...> {
            using type = TypeList<UpdatedLevel, Ls...>;
        };

        template <std::size_t I, class L0, class... Ls>
        struct ReplaceLevel<I, L0, Ls...> {
            using type = PushBack_t<typename ReplaceLevel<I - 1, Ls...>::type, L0>;
        };

        using type = typename ReplaceLevel<Index, Levels...>::type;
    };

    template <std::size_t Index>
    struct AddImpl<false, Index> {
        using type = typename InsertLevel<NewLevel, TypeList<Levels...>, IsDescending>::type;
    };

    using new_levels_type = typename AddImpl<found, found_index>::type;

    template <class T>
    struct ToSideBook;

    template <class... Ls>
    struct ToSideBook<TypeList<Ls...>> {
        using type = SideBook<Ls...>;
    };

public:
    using type = typename ToSideBook<new_levels_type>::type;
};

template <bool IsDescending, class NewOrder>
requires ValidOrder<NewOrder>
struct AddOrderToSide<IsDescending, SideBook<>, NewOrder> {
    using type = SideBook<PriceLevel<NewOrder::price, NewOrder>>;
};

template <bool IsDescending, class Book, class NewOrder>
requires ValidOrder<NewOrder>
using AddOrderToSide_t = typename AddOrderToSide<IsDescending, Book, NewOrder>::type;

template <class Book>
struct RemoveEmptyLevels;

template <class... Levels>
struct RemoveEmptyLevels<SideBook<Levels...>> {
private:
    template <class Level>
    struct IsNotEmpty {
        static constexpr bool value = !Level::is_empty;
    };
    using filtered = Filter_t<TypeList<Levels...>, IsNotEmpty>;

    template <class T>
    struct ToSideBook;

    template <class... Ls>
    struct ToSideBook<TypeList<Ls...>> {
        using type = SideBook<Ls...>;
    };

public:
    using type = typename ToSideBook<filtered>::type;
};

template <class Book>
using RemoveEmptyLevels_t = typename RemoveEmptyLevels<Book>::type;

template <class Level, spec::OrderId Id>
struct RemoveOrderFromLevel;

template <spec::Price PriceV, spec::OrderId Id>
struct RemoveOrderFromLevel<PriceLevel<PriceV>, Id> {
    using type = PriceLevel<PriceV>;
    static constexpr bool removed = false;
};

template <spec::Price PriceV, class Head, class... Tail, spec::OrderId Id>
struct RemoveOrderFromLevel<PriceLevel<PriceV, Head, Tail...>, Id> {
private:
    using tail_result = RemoveOrderFromLevel<PriceLevel<PriceV, Tail...>, Id>;
    using kept_tail = typename tail_result::type;

    template <class Result>
    struct PrependHead;

    template <spec::Price ResultPrice, class... Orders>
    struct PrependHead<PriceLevel<ResultPrice, Orders...>> {
        using type = PriceLevel<ResultPrice, Head, Orders...>;
    };

public:
    using type = std::conditional_t<
        Head::id == Id,
        kept_tail,
        typename PrependHead<kept_tail>::type
    >;
    static constexpr bool removed = (Head::id == Id) || tail_result::removed;
};

template <class Side, spec::OrderId Id>
struct CancelOrderFromSide;

template <spec::OrderId Id>
struct CancelOrderFromSide<SideBook<>, Id> {
    using type = SideBook<>;
};

template <class First, class... Rest, spec::OrderId Id>
struct CancelOrderFromSide<SideBook<First, Rest...>, Id> {
private:
    using level_result = RemoveOrderFromLevel<First, Id>;
    using updated_level = typename level_result::type;

    template <class Level, class Tail>
    struct Rebuild;

    template <class Level, class... Levels>
    struct Rebuild<Level, SideBook<Levels...>> {
        using type = SideBook<Level, Levels...>;
    };

    using rest_result = typename CancelOrderFromSide<SideBook<Rest...>, Id>::type;
    using updated_side = typename Rebuild<updated_level, rest_result>::type;

public:
    using type = RemoveEmptyLevels_t<updated_side>;
};

template <class Side, spec::OrderId Id>
using CancelOrderFromSide_t = typename CancelOrderFromSide<Side, Id>::type;

template <class Book>
constexpr bool side_book_is_empty_v = Book::is_empty;

template <class Book>
constexpr spec::Price side_book_best_price_v = Book::best_price;

template <class Book>
using SideBookBestLevel_t = typename Book::best_level;

// BidBook uses descending order (highest price first)
template <class... Levels>
using BidBook = SideBook<Levels...>;

// AskBook uses ascending order (lowest price first)
template <class... Levels>
using AskBook = SideBook<Levels...>;

template <class Book, class NewOrder>
requires ValidOrder<NewOrder>
using AddOrderToBidBook_t = AddOrderToSide_t<true, Book, NewOrder>;

template <class Book, class NewOrder>
requires ValidOrder<NewOrder>
using AddOrderToAskBook_t = AddOrderToSide_t<false, Book, NewOrder>;

}