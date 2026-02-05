#pragma once

#include "concepts.hpp"
#include "order.hpp"
#include "type_list.hpp"
#include "../lob/spec.hpp"
#include <array>
#include <optional>
#include <type_traits>

namespace lob::compile_time {

template <ValidBook BookT>
struct RuntimeBookView;

template <ValidBook NewBook, class TradeList, class Remainder>
struct MatchResult {
    using book = NewBook;
    using trades = TradeList;
    using remainder = Remainder;

    static constexpr std::size_t trade_count = TradeList::size;

    template <class Sink>
    static constexpr void apply_trades(Sink&& sink) noexcept {
        apply_trades_impl(TradeList{}, std::forward<Sink>(sink));
    }

    static constexpr auto runtime_view() noexcept {
        return RuntimeBookView<NewBook>::from_book();
    }

private:
    template <class... TradesT, class Sink>
    static constexpr void apply_trades_impl(TypeList<TradesT...>, Sink&& sink) noexcept {
        (sink.on_trade(spec::Trade{
            TradesT::aggressor,
            TradesT::resting,
            TradesT::price,
            TradesT::quantity
        }), ...);
    }
};

template <ValidBook BookT>
struct RuntimeBookView {
    struct LevelView {
        spec::Price price;
        spec::Quantity total_quantity;
        std::size_t order_count;
    };

    template <std::size_t Capacity>
    struct SideView {
        std::size_t level_count = 0;
        std::array<LevelView, Capacity> levels{};
    };

    SideView<BookT::bid_level_count> bids;
    SideView<BookT::ask_level_count> asks;

    static constexpr RuntimeBookView from_book() noexcept {
        RuntimeBookView view{};

        if constexpr (!BookT::bids_empty) {
            view.bids.level_count = BookT::bid_level_count;
            fill_levels_impl<0>(view.bids, typename BookT::bids::levels{});
        }

        if constexpr (!BookT::asks_empty) {
            view.asks.level_count = BookT::ask_level_count;
            fill_levels_impl<0>(view.asks, typename BookT::asks::levels{});
        }

        return view;
    }

private:
    template <std::size_t I, std::size_t Capacity, class LevelList>
    static constexpr void fill_levels_impl(SideView<Capacity>& side, LevelList) noexcept {
        if constexpr (I < ListSize<LevelList>::value) {
            using Level = At_t<I, LevelList>;
            side.levels[I] = LevelView{
                Level::price,
                Level::total_quantity,
                Level::order_count
            };
            fill_levels_impl<I + 1>(side, LevelList{});
        }
    }
};

template <ValidBook BookT>
constexpr auto runtime_view_v = RuntimeBookView<BookT>::from_book();

}