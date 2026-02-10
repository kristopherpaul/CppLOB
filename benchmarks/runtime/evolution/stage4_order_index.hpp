#pragma once

#include <array>
#include <optional>
#include <cstdint>
#include "../../../include/lob/spec.hpp"
#include "../../../include/lob/rules.hpp"

#include <vector>
namespace lob::evolution {

struct OrderNode {
    spec::OrderId id = 0;
    spec::Price price = 0;
    spec::Quantity quantity = 0;
    std::uint32_t prev = UINT32_MAX;
    std::uint32_t next = UINT32_MAX;
    std::uint32_t next_free = UINT32_MAX;
    spec::Side side = spec::Side::Buy;
    bool active = false;
};

struct PriceLevel {
    std::uint32_t head = UINT32_MAX;
    std::uint32_t tail = UINT32_MAX;
    spec::Quantity total_quantity = 0;
    bool empty() const noexcept { return head == UINT32_MAX; }
    void reset() noexcept { head = tail = UINT32_MAX; total_quantity = 0; }
};

template <std::size_t MaxOrders>
class OrderPool {
    std::array<OrderNode, MaxOrders> nodes_;
    std::uint32_t free_head_ = 0;
    std::uint32_t free_count_ = MaxOrders;

public:
    OrderPool() noexcept {
        for (std::size_t i = 0; i < MaxOrders; ++i) {
            nodes_[i].next_free = (i + 1 < MaxOrders) ? static_cast<std::uint32_t>(i + 1) : UINT32_MAX;
            nodes_[i].active = false;
        }
    }

    std::uint32_t allocate() noexcept {
        if (free_head_ == UINT32_MAX) return UINT32_MAX;
        std::uint32_t idx = free_head_;
        free_head_ = nodes_[idx].next_free;
        nodes_[idx].active = true;
        nodes_[idx].next_free = UINT32_MAX;
        --free_count_;
        return idx;
    }

    void deallocate(std::uint32_t idx) noexcept {
        nodes_[idx].active = false;
        nodes_[idx].next_free = free_head_;
        free_head_ = idx;
        ++free_count_;
    }

    OrderNode& operator[](std::uint32_t idx) noexcept { return nodes_[idx]; }
    const OrderNode& operator[](std::uint32_t idx) const noexcept { return nodes_[idx]; }
    std::uint32_t free_count() const noexcept { return free_count_; }
    bool full() const noexcept { return free_head_ == UINT32_MAX; }
    void clear() noexcept {
        for (std::size_t i = 0; i < MaxOrders; ++i) {
            nodes_[i].next_free = (i + 1 < MaxOrders) ? static_cast<std::uint32_t>(i + 1) : UINT32_MAX;
            nodes_[i].active = false;
        }
        free_head_ = 0;
        free_count_ = MaxOrders;
    }
};

template <std::size_t MaxOrders>
class OrderIndex {
    struct Entry {
        spec::OrderId id = 0;
        std::uint32_t pool_index = UINT32_MAX;
        std::uint8_t state = 0;  // 0=empty, 1=occupied, 2=tombstone
    };

    static constexpr std::size_t capacity = []() consteval {
        std::size_t cap = MaxOrders * 2;
        cap |= cap >> 1;
        cap |= cap >> 2;
        cap |= cap >> 4;
        cap |= cap >> 8;
        cap |= cap >> 16;
        cap |= cap >> 32;
        return cap + 1;
    }();

    std::array<Entry, capacity> entries_;
    std::size_t size_ = 0;

    constexpr std::size_t hash(spec::OrderId id) const noexcept {
        return static_cast<std::size_t>(id * 11400714819323198485ull) & (capacity - 1);
    }

    std::size_t find_slot(spec::OrderId id) const noexcept {
        std::size_t slot = hash(id);
        for (std::size_t probes = 0; probes < capacity; ++probes) {
            if (entries_[slot].state == 0) return capacity;
            if (entries_[slot].state == 1 && entries_[slot].id == id) return slot;
            slot = (slot + 1) & (capacity - 1);
        }
        return capacity;
    }

    std::size_t find_slot_for_insert(spec::OrderId id) const noexcept {
        std::size_t slot = hash(id);
        std::size_t first_tombstone = capacity;
        for (std::size_t probes = 0; probes < capacity; ++probes) {
            if (entries_[slot].state == 0) {
                return first_tombstone == capacity ? slot : first_tombstone;
            }
            if (entries_[slot].state == 1 && entries_[slot].id == id) return slot;
            if (entries_[slot].state == 2 && first_tombstone == capacity) first_tombstone = slot;
            slot = (slot + 1) & (capacity - 1);
        }
        return first_tombstone;
    }

    std::size_t find_slot_for_erase(spec::OrderId id) const noexcept {
        std::size_t slot = hash(id);
        for (std::size_t probes = 0; probes < capacity; ++probes) {
            if (entries_[slot].state == 0) return capacity;
            if (entries_[slot].state == 1 && entries_[slot].id == id) return slot;
            slot = (slot + 1) & (capacity - 1);
        }
        return capacity;
    }

public:
    OrderIndex() noexcept = default;

    std::uint32_t find(spec::OrderId id) const noexcept {
        std::size_t slot = find_slot(id);
        if (slot == capacity) return UINT32_MAX;
        return entries_[slot].pool_index;
    }

    bool insert(spec::OrderId id, std::uint32_t pool_index) noexcept {
        if (size_ >= capacity / 2) return false;
        std::size_t slot = find_slot_for_insert(id);
        if (slot == capacity) return false;
        if (entries_[slot].state == 1 && entries_[slot].id == id) return false;
        entries_[slot] = {id, pool_index, 1};
        ++size_;
        return true;
    }

    bool erase(spec::OrderId id) noexcept {
        std::size_t slot = find_slot_for_erase(id);
        if (slot == capacity) return false;
        entries_[slot] = {0, UINT32_MAX, 2};
        --size_;
        return true;
    }

    void clear() noexcept {
        for (auto& e : entries_) e = {0, UINT32_MAX, 0};
        size_ = 0;
    }
};

template <spec::Price MinPrice, spec::Price MaxPrice, std::size_t MaxOrders>
class Stage4_OrderIndex {
    static constexpr std::size_t NumLevels = static_cast<std::size_t>(MaxPrice - MinPrice + 1);
    
    OrderPool<MaxOrders> pool_;
    OrderIndex<MaxOrders> index_;
    std::array<PriceLevel, NumLevels> bids_;
    std::array<PriceLevel, NumLevels> asks_;

    constexpr std::size_t price_to_index(spec::Price price) const noexcept {
        return static_cast<std::size_t>(price - MinPrice);
    }

    constexpr spec::Price index_to_price(std::size_t index) const noexcept {
        return MinPrice + static_cast<spec::Price>(index);
    }

    void append_to_level(PriceLevel& level, std::uint32_t idx) noexcept {
        auto& pool = pool_;
        if (level.empty()) {
            level.head = level.tail = idx;
            pool[idx].prev = UINT32_MAX;
        } else {
            pool[level.tail].next = idx;
            pool[idx].prev = level.tail;
            level.tail = idx;
        }
        pool[idx].next = UINT32_MAX;
    }

    void unlink_from_level(PriceLevel& level, std::uint32_t idx, spec::Quantity qty) noexcept {
        auto& pool = pool_;
        auto& node = pool[idx];
        if (node.prev != UINT32_MAX) {
            pool[node.prev].next = node.next;
        } else {
            level.head = node.next;
        }
        if (node.next != UINT32_MAX) {
            pool[node.next].prev = node.prev;
        } else {
            level.tail = node.prev;
        }
        level.total_quantity -= qty;
        if (level.empty()) level.reset();
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

    bool submit(const spec::OrderInput& in, TradeSink& sink) noexcept {
        if (!spec::valid_order_input(in)) return false;
        if (in.price < MinPrice || in.price > MaxPrice) return false;
        if (pool_.full()) return false;

        std::uint32_t pool_idx = pool_.allocate();
        if (pool_idx == UINT32_MAX) return false;

        auto& node = pool_[pool_idx];
        node.id = in.id;
        node.price = in.price;
        node.quantity = in.quantity;
        node.side = in.side;
        node.prev = UINT32_MAX;

        if (!index_.insert(in.id, pool_idx)) {
            pool_.deallocate(pool_idx);
            return false;
        }

        if (in.side == spec::Side::Buy) {
            return match_buy(in, pool_idx, sink);
        } else {
            return match_sell(in, pool_idx, sink);
        }
    }

    bool cancel(spec::OrderId id) noexcept {
        std::uint32_t pool_idx = index_.find(id);
        if (pool_idx == UINT32_MAX) return false;

        auto& node = pool_[pool_idx];
        spec::Price price = node.price;
        spec::Quantity qty = node.quantity;
        spec::Side side = node.side;

        auto& ladder = (side == spec::Side::Buy) ? bids_ : asks_;
        auto& level = ladder[price_to_index(price)];
        unlink_from_level(level, pool_idx, qty);
        index_.erase(id);
        pool_.deallocate(pool_idx);
        return true;
    }

    std::optional<spec::Price> best_bid() const noexcept { return best_buy_linear(); }
    std::optional<spec::Price> best_ask() const noexcept { return best_sell_linear(); }

    void clear() noexcept {
        pool_.clear();
        index_.clear();
        for (auto& level : bids_) level.reset();
        for (auto& level : asks_) level.reset();
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
            std::uint32_t resting_idx = level.head;
            if (resting_idx == UINT32_MAX) continue;

            auto& resting = pool_[resting_idx];
            spec::Quantity trade_qty = std::min(remaining, resting.quantity);

            sink.on_trade({in.id, resting.id, resting.price, trade_qty});

            remaining -= trade_qty;
            resting.quantity -= trade_qty;
            level.total_quantity -= trade_qty;

            if (resting.quantity == 0) {
                unlink_from_level(level, resting_idx, trade_qty);
                index_.erase(resting.id);
                pool_.deallocate(resting_idx);
            }
        }

        if (remaining > 0) {
            node.quantity = remaining;
            auto& level = bids_[price_to_index(in.price)];
            append_to_level(level, pool_idx);
            level.total_quantity += remaining;
        } else {
            index_.erase(in.id);
            pool_.deallocate(pool_idx);
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
            std::uint32_t resting_idx = level.head;
            if (resting_idx == UINT32_MAX) continue;

            auto& resting = pool_[resting_idx];
            spec::Quantity trade_qty = std::min(remaining, resting.quantity);

            sink.on_trade({in.id, resting.id, resting.price, trade_qty});

            remaining -= trade_qty;
            resting.quantity -= trade_qty;
            level.total_quantity -= trade_qty;

            if (resting.quantity == 0) {
                unlink_from_level(level, resting_idx, trade_qty);
                index_.erase(resting.id);
                pool_.deallocate(resting_idx);
            }
        }

        if (remaining > 0) {
            node.quantity = remaining;
            auto& level = asks_[price_to_index(in.price)];
            append_to_level(level, pool_idx);
            level.total_quantity += remaining;
        } else {
            index_.erase(in.id);
            pool_.deallocate(pool_idx);
        }
        return true;
    }
};

}