#pragma once

#include "config.hpp"
#include "order_pool.hpp"
#include <cstdint>

namespace lob::runtime {

struct PriceLevel {
    std::uint32_t head = npos;
    std::uint32_t tail = npos;
    spec::Quantity total_quantity = 0;

    constexpr bool empty() const noexcept {
        return head == npos;
    }

    constexpr void reset() noexcept {
        head = npos;
        tail = npos;
        total_quantity = 0;
    }
};

template <class Config>
inline void append_to_level(PriceLevel& level, std::uint32_t order_index, 
                             std::array<OrderNode, Config::max_orders>& pool) noexcept {
    if (level.empty()) {
        level.head = level.tail = order_index;
    } else {
        pool[level.tail].next = order_index;
        pool[order_index].prev = level.tail;
        level.tail = order_index;
    }
}

template <class Config>
inline void unlink_from_level(PriceLevel& level, std::uint32_t order_index,
                               std::array<OrderNode, Config::max_orders>& pool,
                               spec::Quantity order_qty) noexcept {
    auto& node = pool[order_index];
    
    if (node.prev != npos) {
        pool[node.prev].next = node.next;
    } else {
        level.head = node.next;
    }

    if (node.next != npos) {
        pool[node.next].prev = node.prev;
    } else {
        level.tail = node.prev;
    }

    level.total_quantity -= order_qty;
    
    if (level.empty()) {
        level.reset();
    }
}

}