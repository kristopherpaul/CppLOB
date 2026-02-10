#pragma once

#include <array>
#include <vector>
#include <optional>
#include <cstdint>
#include "../../../include/lob/spec.hpp"
#include "../../../include/lob/rules.hpp"

namespace lob::evolution {

struct OrderNode {
    spec::OrderId id = 0;
    spec::Price price = 0;
    spec::Quantity quantity = 0;
    spec::Side side = spec::Side::Buy;
    bool active = false;
};

struct PriceLevel {
    std::vector<std::uint32_t> order_indices;  // indices into pool
    spec::Quantity total_quantity = 0;
    bool empty() const noexcept { return order_indices.empty(); }
    void reset() noexcept { order_indices.clear(); total_quantity = 0; }
};

template <spec::Price MinPrice, spec::Price MaxPrice, std::size_t MaxOrders>
class Stage1_FlatLadder {
    static constexpr std::size_t NumLevels = static_cast<std::size_t>(MaxPrice - MinPrice + 1);
    
    std::array<OrderNode, MaxOrders> pool_;
    std::array<PriceLevel, NumLevels> bids_;
    std::array<PriceLevel, NumLevels> asks_;
    std::uint32_t next_free_ = 0;

    constexpr std::size_t price_to_index(spec::Price price) const noexcept {
        return static_cast<std::size_t>(price - MinPrice);
    }

    constexpr spec::Price index_to_price(std::size_t index) const noexcept {
        return MinPrice + static_cast<spec::Price>(index);
    }

    std::uint32_t allocate() noexcept {
        if (next_free_ >= MaxOrders) return UINT32_MAX;
        std::uint32_t idx = next_free_++;
        pool_[idx].active = true;
        return idx;
    }

    void deallocate(std::uint32_t idx) noexcept {
        pool_[idx].active = false;
        // No free list in stage 1
    }

    std::optional<spec::Price> best_buy_linear() const noexcept {
        for (std::size_t i = NumLevels; i-- > 0;) {
            if (!bids_[i].empty()) return index_to_price(i);
        }
        return std::nullopt;
    }

    std::optional<spec::Price> best_sell_linear() const noexcept {
        for (std::size_t i = 0; i < NumLevels; ++i) {
            if (!asks_[i].empty()) return index_to_price(i);
        }
        return std::nullopt;
    }

public:
    struct TradeSink {
        std::vector<spec::Trade>& trades;
        void on_trade(const spec::Trade& t) noexcept { trades.push_back(t); }
    };

    Stage1_FlatLadder() noexcept = default;

    bool submit(const spec::OrderInput& in, TradeSink& sink) noexcept {
        if (!spec::valid_order_input(in)) return false;
        if (in.price < MinPrice || in.price > MaxPrice) return false;
        if (next_free_ >= MaxOrders) return false;

        std::uint32_t pool_idx = allocate();
        if (pool_idx == UINT32_MAX) return false;

        auto& node = pool_[pool_idx];
        node.id = in.id;
        node.price = in.price;
        node.quantity = in.quantity;
        node.side = in.side;

        if (in.side == spec::Side::Buy) {
            return match_buy(in, pool_idx, sink);
        } else {
            return match_sell(in, pool_idx, sink);
        }
    }

    bool cancel(spec::OrderId id) noexcept {
        for (std::uint32_t i = 0; i < next_free_; ++i) {
            if (pool_[i].active && pool_[i].id == id) {
                auto& node = pool_[i];
                spec::Price price = node.price;
                spec::Quantity qty = node.quantity;
                spec::Side side = node.side;

                auto& ladder = (side == spec::Side::Buy) ? bids_ : asks_;
                auto& level = ladder[price_to_index(price)];
                
                // Find and remove from vector (O(n) at level)
                auto it = std::find(level.order_indices.begin(), level.order_indices.end(), i);
                if (it != level.order_indices.end()) {
                    level.order_indices.erase(it);
                    level.total_quantity -= qty;
                    deallocate(i);
                    return true;
                }
            }
        }
        return false;
    }

    std::optional<spec::Price> best_bid() const noexcept { return best_buy_linear(); }
    std::optional<spec::Price> best_ask() const noexcept { return best_sell_linear(); }

    void clear() noexcept {
        for (auto& level : bids_) level.reset();
        for (auto& level : asks_) level.reset();
        for (std::size_t i = 0; i < MaxOrders; ++i) pool_[i].active = false;
        next_free_ = 0;
    }

private:
    bool match_buy(const spec::OrderInput& in, std::uint32_t pool_idx, TradeSink& sink) noexcept {
        auto& node = pool_[pool_idx];
        spec::Quantity remaining = in.quantity;

        while (remaining > 0) {
            auto opp_best = best_sell_linear();
            if (!opp_best) break;
            if (!spec::crosses(spec::Side::Buy, in.price, *opp_best)) break;

            spec::Price opp_price = *opp_best;
            auto& level = asks_[price_to_index(opp_price)];
            if (level.empty()) continue;

            std::uint32_t resting_idx = level.order_indices.front();
            auto& resting = pool_[resting_idx];
            spec::Quantity trade_qty = std::min(remaining, resting.quantity);

            sink.on_trade({in.id, resting.id, resting.price, trade_qty});

            remaining -= trade_qty;
            resting.quantity -= trade_qty;
            level.total_quantity -= trade_qty;

            if (resting.quantity == 0) {
                level.order_indices.erase(level.order_indices.begin());
                deallocate(resting_idx);
            }
        }

        if (remaining > 0) {
            node.quantity = remaining;
            auto& level = bids_[price_to_index(in.price)];
            level.order_indices.push_back(pool_idx);
            level.total_quantity += remaining;
        } else {
            deallocate(pool_idx);
        }
        return true;
    }

    bool match_sell(const spec::OrderInput& in, std::uint32_t pool_idx, TradeSink& sink) noexcept {
        auto& node = pool_[pool_idx];
        spec::Quantity remaining = in.quantity;

        while (remaining > 0) {
            auto opp_best = best_buy_linear();
            if (!opp_best) break;
            if (!spec::crosses(spec::Side::Sell, in.price, *opp_best)) break;

            spec::Price opp_price = *opp_best;
            auto& level = bids_[price_to_index(opp_price)];
            if (level.empty()) continue;

            std::uint32_t resting_idx = level.order_indices.front();
            auto& resting = pool_[resting_idx];
            spec::Quantity trade_qty = std::min(remaining, resting.quantity);

            sink.on_trade({in.id, resting.id, resting.price, trade_qty});

            remaining -= trade_qty;
            resting.quantity -= trade_qty;
            level.total_quantity -= trade_qty;

            if (resting.quantity == 0) {
                level.order_indices.erase(level.order_indices.begin());
                deallocate(resting_idx);
            }
        }

        if (remaining > 0) {
            node.quantity = remaining;
            auto& level = asks_[price_to_index(in.price)];
            level.order_indices.push_back(pool_idx);
            level.total_quantity += remaining;
        } else {
            deallocate(pool_idx);
        }
        return true;
    }
};

}