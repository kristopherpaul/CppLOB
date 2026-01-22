#pragma once

#include "config.hpp"
#include "price_ladder.hpp"
#include "order_pool.hpp"
#include "order_index.hpp"
#include "bitmap.hpp"
#include "price_level.hpp"
#include "../lob/spec.hpp"
#include "../lob/rules.hpp"

namespace lob::runtime {

template <class Config>
class SideBook {
    PriceLadder<Config> ladder_;
    BitmapIndex<Config> bitmap_;

public:
    constexpr SideBook() noexcept = default;

    constexpr PriceLadder<Config>& ladder() noexcept { return ladder_; }
    constexpr const PriceLadder<Config>& ladder() const noexcept { return ladder_; }
    constexpr BitmapIndex<Config>& bitmap() noexcept { return bitmap_; }
    constexpr const BitmapIndex<Config>& bitmap() const noexcept { return bitmap_; }

    constexpr void clear() noexcept {
        ladder_.clear();
        bitmap_.clear();
    }
};

template <class Config, class Pool, class Index>
class MatchEngine {
    SideBook<Config> bids_;
    SideBook<Config> asks_;
    Pool& pool_;
    Index& index_;

    template <spec::Side Side>
    constexpr SideBook<Config>& side() noexcept {
        if constexpr (Side == spec::Side::Buy) return bids_;
        else return asks_;
    }

    template <spec::Side Side>
    constexpr const SideBook<Config>& side() const noexcept {
        if constexpr (Side == spec::Side::Buy) return bids_;
        else return asks_;
    }

    template <spec::Side Side>
    constexpr SideBook<Config>& opposite_side() noexcept {
        if constexpr (Side == spec::Side::Buy) return asks_;
        else return bids_;
    }

    template <spec::Side Side>
    constexpr const SideBook<Config>& opposite_side() const noexcept {
        if constexpr (Side == spec::Side::Buy) return asks_;
        else return bids_;
    }

public:
    constexpr MatchEngine(Pool& pool, Index& index) noexcept : pool_(pool), index_(index) {}

    constexpr SideBook<Config>& bids() noexcept { return bids_; }
    constexpr SideBook<Config>& asks() noexcept { return asks_; }
    constexpr const SideBook<Config>& bids() const noexcept { return bids_; }
    constexpr const SideBook<Config>& asks() const noexcept { return asks_; }

    template <class TradeSink>
    constexpr bool submit_order(const spec::OrderInput& order, TradeSink& sink) noexcept {
        if (!Config::valid_price(order.price)) return false;
        if (!spec::valid_quantity(order.quantity)) return false;
        if (pool_.full()) return false;

        std::uint32_t pool_index = pool_.allocate();
        if (pool_index == npos) return false;

        auto& node = pool_[pool_index];
        node.id = order.id;
        node.price = order.price;
        node.quantity = order.quantity;
        node.side = order.side;
        node.prev = npos;
        node.next = npos;
        node.state = 1;

        if (!index_.insert(order.id, pool_index)) {
            pool_.deallocate(pool_index);
            return false;
        }

        if (order.side == spec::Side::Buy) {
            return match<spec::Side::Buy>(order, pool_index, sink);
        } else {
            return match<spec::Side::Sell>(order, pool_index, sink);
        }
    }

    template <spec::Side Side, class TradeSink>
    constexpr bool match(const spec::OrderInput& incoming, std::uint32_t pool_index, TradeSink& sink) noexcept {
        auto& node = pool_[pool_index];
        spec::Quantity remaining = incoming.quantity;

        while (remaining > 0) {
            auto opp_best = opposite_side<Side>().bitmap().template best_price<spec::opposite(Side)>();
            if (!opp_best) break;

            spec::Price opp_price = *opp_best;
            if (!spec::crosses(Side, incoming.price, opp_price)) break;

            auto& opp_ladder = opposite_side<Side>().ladder();
            auto& opp_level = opp_ladder.level(opp_price);
            std::uint32_t resting_index = opp_level.head;
            
            if (resting_index == npos) {
                opposite_side<Side>().bitmap().clear_bit(Config::price_to_index(opp_price));
                continue;
            }

            auto& resting = pool_[resting_index];
            spec::Quantity trade_qty = remaining < resting.quantity ? remaining : resting.quantity;

            sink.on_trade({incoming.id, resting.id, resting.price, trade_qty});

            remaining -= trade_qty;
            resting.quantity -= trade_qty;
            opp_level.total_quantity -= trade_qty;

            if (resting.quantity == 0) {
                unlink_from_level<Config>(opp_level, resting_index, pool_.nodes(), trade_qty);
                index_.erase(resting.id);
                pool_.deallocate(resting_index);

                if (opp_level.empty()) {
                    opposite_side<Side>().bitmap().clear_bit(Config::price_to_index(opp_price));
                }
            }
        }

        if (remaining > 0) {
            node.quantity = remaining;
            auto& my_ladder = side<Side>().ladder();
            auto& my_level = my_ladder.level(incoming.price);
            append_to_level<Config>(my_level, pool_index, pool_.nodes());
            my_level.total_quantity += remaining;
            side<Side>().bitmap().set(Config::price_to_index(incoming.price));
        } else {
            index_.erase(incoming.id);
            pool_.deallocate(pool_index);
        }

        return true;
    }

    constexpr bool cancel(spec::OrderId id) noexcept {
        std::uint32_t pool_index = index_.find(id);
        if (pool_index == npos) return false;

        auto& node = pool_[pool_index];
        spec::Side side = node.side;
        spec::Price price = node.price;
        spec::Quantity qty = node.quantity;

        auto& my_ladder = (side == spec::Side::Buy) ? bids_.ladder() : asks_.ladder();
        auto& my_level = my_ladder.level(price);
        unlink_from_level<Config>(my_level, pool_index, pool_.nodes(), qty);

        if (my_level.empty()) {
            (side == spec::Side::Buy ? bids_.bitmap() : asks_.bitmap()).clear_bit(Config::price_to_index(price));
        }

        index_.erase(id);
        pool_.deallocate(pool_index);
        return true;
    }

    constexpr std::optional<spec::Price> best_bid() const noexcept {
        return bids_.bitmap().best_buy();
    }

    constexpr std::optional<spec::Price> best_ask() const noexcept {
        return asks_.bitmap().best_sell();
    }

    constexpr void clear() noexcept {
        bids_.clear();
        asks_.clear();
        pool_.clear();
        index_.clear();
    }
};

}