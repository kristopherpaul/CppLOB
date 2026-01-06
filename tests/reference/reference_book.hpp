#pragma once

#include "../../include/lob/spec.hpp"
#include "../../include/lob/rules.hpp"
#include <map>
#include <deque>
#include <vector>
#include <optional>
#include <unordered_map>

namespace lob::reference {

struct Order {
    spec::OrderId id;
    spec::Side side;
    spec::Price price;
    spec::Quantity quantity;
};

class ReferenceBook {
    using Level = std::deque<Order>;
    std::map<spec::Price, Level, std::greater<>> bids_;
    std::map<spec::Price, Level> asks_;
    std::unordered_map<spec::OrderId, std::pair<spec::Side, spec::Price>> index_;

public:
    struct Snapshot {
        std::map<spec::Price, std::vector<Order>, std::greater<>> bids;
        std::map<spec::Price, std::vector<Order>> asks;
    };

    bool submit(const spec::OrderInput& in) {
        if (!spec::valid_order_input(in)) return false;
        if (index_.contains(in.id)) return false;

        Order order{in.id, in.side, in.price, in.quantity};

        if (in.side == spec::Side::Buy) {
            return submit_buy(order);
        } else {
            return submit_sell(order);
        }
    }

    bool cancel(spec::OrderId id) {
        auto it = index_.find(id);
        if (it == index_.end()) return false;

        auto [side, price] = it->second;

        if (side == spec::Side::Buy) {
            auto level_it = bids_.find(price);
            if (level_it == bids_.end()) return false;
            auto& level = level_it->second;
            for (auto oit = level.begin(); oit != level.end(); ++oit) {
                if (oit->id == id) {
                    level.erase(oit);
                    break;
                }
            }
            if (level.empty()) {
                bids_.erase(level_it);
            }
        } else {
            auto level_it = asks_.find(price);
            if (level_it == asks_.end()) return false;
            auto& level = level_it->second;
            for (auto oit = level.begin(); oit != level.end(); ++oit) {
                if (oit->id == id) {
                    level.erase(oit);
                    break;
                }
            }
            if (level.empty()) {
                asks_.erase(level_it);
            }
        }

        index_.erase(it);
        return true;
    }

    std::optional<spec::Price> best_bid() const noexcept {
        if (bids_.empty()) return std::nullopt;
        return bids_.begin()->first;
    }

    std::optional<spec::Price> best_ask() const noexcept {
        if (asks_.empty()) return std::nullopt;
        return asks_.begin()->first;
    }

    Snapshot snapshot() const {
        Snapshot snap;
        for (auto& [price, level] : bids_) {
            snap.bids[price] = std::vector<Order>(level.begin(), level.end());
        }
        for (auto& [price, level] : asks_) {
            snap.asks[price] = std::vector<Order>(level.begin(), level.end());
        }
        return snap;
    }

    const std::vector<spec::Trade>& trades() const noexcept { return trades_; }
    void clear_trades() { trades_.clear(); }

    bool empty() const noexcept { return bids_.empty() && asks_.empty(); }
    std::size_t bid_levels() const noexcept { return bids_.size(); }
    std::size_t ask_levels() const noexcept { return asks_.size(); }

private:
    bool submit_buy(Order& order) {
        while (order.quantity > 0 && !asks_.empty()) {
            auto opp_it = asks_.begin();
            spec::Price opp_price = opp_it->first;

            if (!spec::crosses(spec::Side::Buy, order.price, opp_price)) break;

            auto& level = opp_it->second;
            Order& resting = level.front();

            spec::Quantity trade_qty = std::min(order.quantity, resting.quantity);

            order.quantity -= trade_qty;
            resting.quantity -= trade_qty;

            trades_.push_back({order.id, resting.id, resting.price, trade_qty});

            if (resting.quantity == 0) {
                index_.erase(resting.id);
                level.pop_front();
            }

            if (level.empty()) {
                asks_.erase(opp_it);
            }
        }

        if (order.quantity > 0) {
            bids_[order.price].push_back(order);
            index_[order.id] = {spec::Side::Buy, order.price};
        }

        return true;
    }

    bool submit_sell(Order& order) {
        while (order.quantity > 0 && !bids_.empty()) {
            auto opp_it = bids_.begin();
            spec::Price opp_price = opp_it->first;

            if (!spec::crosses(spec::Side::Sell, order.price, opp_price)) break;

            auto& level = opp_it->second;
            Order& resting = level.front();

            spec::Quantity trade_qty = std::min(order.quantity, resting.quantity);

            order.quantity -= trade_qty;
            resting.quantity -= trade_qty;

            trades_.push_back({order.id, resting.id, resting.price, trade_qty});

            if (resting.quantity == 0) {
                index_.erase(resting.id);
                level.pop_front();
            }

            if (level.empty()) {
                bids_.erase(opp_it);
            }
        }

        if (order.quantity > 0) {
            asks_[order.price].push_back(order);
            index_[order.id] = {spec::Side::Sell, order.price};
        }

        return true;
    }

    std::vector<spec::Trade> trades_;
};

}