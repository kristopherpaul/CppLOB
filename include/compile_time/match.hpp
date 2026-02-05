#pragma once

#include "book.hpp"
#include "result.hpp"
#include "trade.hpp"
#include <type_traits>

namespace lob::compile_time {

template <class SideBookT, class NewLevel>
struct ReplaceBestLevel;

template <class First, class... Rest, class NewLevel>
struct ReplaceBestLevel<SideBook<First, Rest...>, NewLevel> {
    using type = SideBook<NewLevel, Rest...>;
};

template <bool IncomingBuy, class Bids, class Asks, class NewOpposite>
struct ReplaceOppositeBook;

template <class Bids, class Asks, class NewOpposite>
struct ReplaceOppositeBook<true, Bids, Asks, NewOpposite> {
    using type = Book<Bids, NewOpposite>;
};

template <class Bids, class Asks, class NewOpposite>
struct ReplaceOppositeBook<false, Bids, Asks, NewOpposite> {
    using type = Book<NewOpposite, Asks>;
};

template <class BookT, class IncomingOrder, class Trades, bool Crosses>
struct MatchStep;

template <ValidBook BookT, ValidOrder IncomingOrder, class Trades>
struct MatchStep<BookT, IncomingOrder, Trades, false> {
private:
    using Bids = typename BookT::bids;
    using Asks = typename BookT::asks;
    using NewBids = std::conditional_t<
        IncomingOrder::is_buy,
        AddOrderToSide_t<true, Bids, IncomingOrder>,
        Bids
    >;
    using NewAsks = std::conditional_t<
        IncomingOrder::is_sell,
        AddOrderToSide_t<false, Asks, IncomingOrder>,
        Asks
    >;

public:
    using type = MatchResult<Book<NewBids, NewAsks>, Trades, IncomingOrder>;
};

template <ValidBook BookT, class IncomingOrder, class RestingRemaining, class Trades,
          bool IncomingFilled, bool RestingFilled>
struct CrossResult;

template <ValidBook BookT, ValidOrder IncomingOrder, class Trades>
struct MatchStep<BookT, IncomingOrder, Trades, true> {
private:
    using Bids = typename BookT::bids;
    using Asks = typename BookT::asks;
    using Opposite = std::conditional_t<IncomingOrder::is_buy, Asks, Bids>;
    using RestingOrder = typename Opposite::best_level::best_order;

    static constexpr spec::Quantity trade_quantity =
        IncomingOrder::quantity < RestingOrder::quantity
            ? IncomingOrder::quantity
            : RestingOrder::quantity;
    static constexpr spec::Price trade_price = RestingOrder::price;

    using IncomingRemaining = OrderWithRemaining_t<IncomingOrder, trade_quantity>;
    using RestingRemaining = OrderWithRemaining_t<RestingOrder, trade_quantity>;
    using CurrentTrade = Trade<
        IncomingOrder::id,
        RestingOrder::id,
        trade_price,
        trade_quantity
    >;
    using UpdatedTrades = PushBack_t<Trades, CurrentTrade>;

public:
    using type = typename CrossResult<
        BookT,
        IncomingRemaining,
        RestingRemaining,
        UpdatedTrades,
        IncomingRemaining::is_filled,
        RestingRemaining::is_filled
    >::type;
};

template <ValidBook BookT, class IncomingOrder, class RestingRemaining, class Trades>
struct CrossResult<BookT, IncomingOrder, RestingRemaining, Trades, true, true> {
private:
    using Bids = typename BookT::bids;
    using Asks = typename BookT::asks;
    using Opposite = std::conditional_t<IncomingOrder::is_buy, Asks, Bids>;
    using RemovedLevel = LevelPopFrontN_t<typename Opposite::best_level, 1>;
    using NewOpposite = RemoveEmptyLevels_t<
        typename ReplaceBestLevel<Opposite, RemovedLevel>::type
    >;
    using NewBook = typename ReplaceOppositeBook<
        IncomingOrder::is_buy,
        Bids,
        Asks,
        NewOpposite
    >::type;

public:
    using type = MatchResult<NewBook, Trades, IncomingOrder>;
};

template <ValidBook BookT, class IncomingOrder, class RestingRemaining, class Trades>
struct CrossResult<BookT, IncomingOrder, RestingRemaining, Trades, true, false> {
private:
    using Bids = typename BookT::bids;
    using Asks = typename BookT::asks;
    using Opposite = std::conditional_t<IncomingOrder::is_buy, Asks, Bids>;
    using RestingOrder = typename Opposite::best_level::best_order;
    using UpdatedLevel = PriceLevel<RestingOrder::price, RestingRemaining>;
    using NewOpposite = typename ReplaceBestLevel<Opposite, UpdatedLevel>::type;
    using NewBook = typename ReplaceOppositeBook<
        IncomingOrder::is_buy,
        Bids,
        Asks,
        NewOpposite
    >::type;

public:
    using type = MatchResult<NewBook, Trades, IncomingOrder>;
};

template <ValidBook BookT, class IncomingOrder, class RestingRemaining, class Trades>
struct CrossResult<BookT, IncomingOrder, RestingRemaining, Trades, false, true> {
private:
    using Bids = typename BookT::bids;
    using Asks = typename BookT::asks;
    using Opposite = std::conditional_t<IncomingOrder::is_buy, Asks, Bids>;
    using RemovedLevel = LevelPopFrontN_t<typename Opposite::best_level, 1>;
    using NewOpposite = RemoveEmptyLevels_t<
        typename ReplaceBestLevel<Opposite, RemovedLevel>::type
    >;
    using NewBook = typename ReplaceOppositeBook<
        IncomingOrder::is_buy,
        Bids,
        Asks,
        NewOpposite
    >::type;

    static constexpr bool crosses_next = []() consteval {
        if constexpr (NewOpposite::is_empty) {
            return false;
        } else {
            return spec::crosses(
                IncomingOrder::side,
                IncomingOrder::price,
                NewOpposite::best_price
            );
        }
    }();

    using Next = MatchStep<NewBook, IncomingOrder, Trades, crosses_next>;

public:
    using type = typename Next::type;
};

template <ValidBook BookT, class IncomingOrder, class RestingRemaining, class Trades>
struct CrossResult<BookT, IncomingOrder, RestingRemaining, Trades, false, false> {
    static_assert(IncomingOrder::quantity == 0, "unreachable fill state");
};

template <ValidStateBook BookT, ValidOrder IncomingOrder>
struct Match {
private:
    using Bids = typename BookT::bids;
    using Asks = typename BookT::asks;
    using Opposite = std::conditional_t<IncomingOrder::is_buy, Asks, Bids>;

    static constexpr bool crosses = []() consteval {
        if constexpr (Opposite::is_empty) {
            return false;
        } else {
            return spec::crosses(
                IncomingOrder::side,
                IncomingOrder::price,
                Opposite::best_price
            );
        }
    }();

public:
    using result = typename MatchStep<
        BookT,
        IncomingOrder,
        TypeList<>,
        crosses
    >::type;
};

template <ValidStateBook BookT, ValidOrder IncomingOrder>
using Submit_t = typename Match<BookT, IncomingOrder>::result;

template <ValidStateBook BookT, ValidOrder NewOrder>
using AddOrder_t = typename Match<BookT, NewOrder>::result::book;

}