#pragma once

#include "side_book.hpp"
#include "order.hpp"
#include "type_list.hpp"
#include "../lob/spec.hpp"
#include "../lob/rules.hpp"
#include <optional>
#include <type_traits>

namespace lob::compile_time {

template <class List, spec::OrderId Id>
struct ContainsOrderId;

template <spec::OrderId Id>
struct ContainsOrderId<TypeList<>, Id> : std::false_type {};

template <class Head, class... Tail, spec::OrderId Id>
struct ContainsOrderId<TypeList<Head, Tail...>, Id>
    : std::bool_constant<(Head::id == Id) || ContainsOrderId<TypeList<Tail...>, Id>::value> {};

template <class LevelList>
struct FlattenLevels;

template <>
struct FlattenLevels<TypeList<>> {
    using type = TypeList<>;
};

template <class Level, class... Rest>
struct FlattenLevels<TypeList<Level, Rest...>> {
    using type = Concat_t<typename Level::orders, typename FlattenLevels<TypeList<Rest...>>::type>;
};

template <class List>
struct UniqueOrderIds;

template <>
struct UniqueOrderIds<TypeList<>> : std::true_type {};

template <class Head, class... Tail>
struct UniqueOrderIds<TypeList<Head, Tail...>>
    : std::bool_constant<
          (!ContainsOrderId<TypeList<Tail...>, Head::id>::value) &&
          UniqueOrderIds<TypeList<Tail...>>::value> {};

template <bool Descending, class... Levels>
struct LevelsSorted;

template <bool Descending>
struct LevelsSorted<Descending> : std::true_type {};

template <bool Descending, class Level>
struct LevelsSorted<Descending, Level>
    : std::bool_constant<LevelContentsValid<Level>::value> {};

template <bool Descending, class First, class Second, class... Rest>
struct LevelsSorted<Descending, First, Second, Rest...>
    : std::bool_constant<
          LevelContentsValid<First>::value &&
          ((Descending && (First::price > Second::price)) ||
           (!Descending && (First::price < Second::price))) &&
          LevelsSorted<Descending, Second, Rest...>::value> {};

template <bool Descending, class List>
struct LevelsSortedFromList;

template <bool Descending, class... Levels>
struct LevelsSortedFromList<Descending, TypeList<Levels...>>
    : LevelsSorted<Descending, Levels...> {};

template <class List, spec::Side SideV>
struct OrdersOnSide;

template <spec::Side SideV>
struct OrdersOnSide<TypeList<>, SideV> : std::true_type {};

template <class Head, class... Tail, spec::Side SideV>
struct OrdersOnSide<TypeList<Head, Tail...>, SideV>
    : std::bool_constant<
          (Head::side == SideV) && OrdersOnSide<TypeList<Tail...>, SideV>::value> {};

template <class Bids, class Asks>
struct Book {
    using bids = Bids;
    using asks = Asks;

    static constexpr bool bids_empty = Bids::is_empty;
    static constexpr bool asks_empty = Asks::is_empty;
    static constexpr bool is_empty = bids_empty && asks_empty;

    static constexpr bool valid = []() consteval {
        if (!Bids::valid || !Asks::valid) {
            return false;
        }
        using BidOrders = typename FlattenLevels<typename Bids::levels>::type;
        using AskOrders = typename FlattenLevels<typename Asks::levels>::type;
        if (!LevelsSortedFromList<true, typename Bids::levels>::value ||
            !LevelsSortedFromList<false, typename Asks::levels>::value) {
            return false;
        }
        if (!OrdersOnSide<BidOrders, spec::Side::Buy>::value ||
            !OrdersOnSide<AskOrders, spec::Side::Sell>::value) {
            return false;
        }
        if (!UniqueOrderIds<Concat_t<BidOrders, AskOrders>>::value) {
            return false;
        }
        if (!bids_empty && !asks_empty) {
            return Bids::best_price < Asks::best_price;
        }
        return true;
    }();

    static constexpr std::optional<spec::Price> best_bid() noexcept {
        if (bids_empty) return std::nullopt;
        return Bids::best_price;
    }

    static constexpr std::optional<spec::Price> best_ask() noexcept {
        if (asks_empty) return std::nullopt;
        return Asks::best_price;
    }

    static constexpr std::size_t bid_level_count = Bids::level_count;
    static constexpr std::size_t ask_level_count = Asks::level_count;
    static constexpr std::size_t total_level_count = bid_level_count + ask_level_count;
};

using EmptyBook = Book<SideBook<>, SideBook<>>;

// Helper for CancelOrder
template <class Bids, class Asks, spec::OrderId CancelId>
struct CancelOrderImpl {
    using NewBids = CancelOrderFromSide_t<Bids, CancelId>;
    using NewAsks = CancelOrderFromSide_t<Asks, CancelId>;
    using type = Book<NewBids, NewAsks>;
};

template <ValidStateBook BookT, spec::OrderId CancelId>
struct CancelOrderHelper {
    using type = typename CancelOrderImpl<typename BookT::bids, typename BookT::asks, CancelId>::type;
};

template <ValidStateBook BookT, spec::OrderId CancelId>
using CancelOrder_t = typename CancelOrderHelper<BookT, CancelId>::type;

}