#pragma once

#include "config.hpp"
#include "order_pool.hpp"
#include "order_index.hpp"
#include "side.hpp"
#include "price_ladder.hpp"
#include "bitmap.hpp"
#include "../lob/spec.hpp"
#include "../lob/concepts.hpp"
#include <array>
#include <optional>
#include <vector>

namespace lob::runtime {

template <class Config>
class OrderBook {
    OrderPool<Config> pool_;
    OrderIndex<Config> index_;
    MatchEngine<Config, OrderPool<Config>, OrderIndex<Config>> engine_;

public:
    constexpr OrderBook() noexcept : engine_(pool_, index_) {}

    template <class TradeSink>
    constexpr bool submit(const spec::OrderInput& order, TradeSink& sink) noexcept {
        return engine_.submit_order(order, sink);
    }

    constexpr bool submit(const spec::OrderInput& order) noexcept {
        struct NullSink {
            void on_trade(const spec::Trade&) noexcept {}
        } null_sink;
        return submit(order, null_sink);
    }

    constexpr bool cancel(spec::OrderId id) noexcept {
        return engine_.cancel(id);
    }

    constexpr std::optional<spec::Price> best_bid() const noexcept {
        return engine_.best_bid();
    }

    constexpr std::optional<spec::Price> best_ask() const noexcept {
        return engine_.best_ask();
    }

    struct Snapshot {
        struct LevelSnapshot {
            spec::Price price;
            spec::Quantity total_quantity;
            std::vector<spec::OrderId> order_ids;
            std::vector<spec::Quantity> quantities;
        };
        std::vector<LevelSnapshot> bids;
        std::vector<LevelSnapshot> asks;
    };

    constexpr Snapshot snapshot() const noexcept {
        Snapshot snap;

        auto append_levels = [this](const auto& ladder, auto& snapshots) {
            for (std::size_t i = 0; i < Config::num_levels; ++i) {
                const auto& level = ladder.level_by_index(i);
                if (level.empty()) continue;

                typename Snapshot::LevelSnapshot snapshot_level;
                snapshot_level.price = Config::index_to_price(i);
                snapshot_level.total_quantity = level.total_quantity;

                for (std::uint32_t idx = level.head; idx != npos; idx = pool_[idx].next) {
                    snapshot_level.order_ids.push_back(pool_[idx].id);
                    snapshot_level.quantities.push_back(pool_[idx].quantity);
                }
                snapshots.push_back(std::move(snapshot_level));
            }
        };

        append_levels(engine_.bids().ladder(), snap.bids);
        append_levels(engine_.asks().ladder(), snap.asks);

        return snap;
    }

    constexpr std::size_t active_order_count() const noexcept {
        return pool_.used_count();
    }

    constexpr std::size_t capacity() const noexcept {
        return Config::max_orders;
    }

    constexpr void clear() noexcept {
        engine_.clear();
    }
};

template <spec::Price MinPrice, spec::Price MaxPrice, std::size_t MaxOrders>
using OrderBookT = OrderBook<RuntimeConfig<MinPrice, MaxPrice, MaxOrders>>;

}
