#pragma once

#include "config.hpp"
#include "../lob/spec.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace lob::runtime {

struct OrderNode {
    spec::OrderId id = 0;
    spec::Price price = 0;
    spec::Quantity quantity = 0;
    std::uint32_t prev = npos;
    std::uint32_t next = npos;
    std::uint32_t next_free = npos;
    spec::Side side = spec::Side::Buy;
    std::uint8_t state = 0; // 0 = free, 1 = active
};

template <class Config>
class OrderPool {
    using Node = OrderNode;
    std::array<Node, Config::max_orders> nodes_;
    std::uint32_t free_head_ = 0;
    std::uint32_t free_count_ = 0;

public:
    constexpr OrderPool() noexcept {
        for (std::size_t i = 0; i < Config::max_orders; ++i) {
            nodes_[i].next_free = (i + 1 < Config::max_orders) ? static_cast<std::uint32_t>(i + 1) : npos;
            nodes_[i].state = 0;
        }
        free_head_ = 0;
        free_count_ = static_cast<std::uint32_t>(Config::max_orders);
    }

    constexpr std::uint32_t allocate() noexcept {
        if (free_head_ == npos) return npos;
        
        std::uint32_t index = free_head_;
        free_head_ = nodes_[index].next_free;
        nodes_[index].state = 1;
        nodes_[index].next_free = npos;
        --free_count_;
        return index;
    }

    constexpr void deallocate(std::uint32_t index) noexcept {
        nodes_[index].state = 0;
        nodes_[index].next_free = free_head_;
        free_head_ = index;
        ++free_count_;
    }

    constexpr Node& operator[](std::uint32_t index) noexcept {
        return nodes_[index];
    }

    constexpr const Node& operator[](std::uint32_t index) const noexcept {
        return nodes_[index];
    }

    constexpr std::uint32_t free_count() const noexcept {
        return free_count_;
    }

    constexpr std::uint32_t used_count() const noexcept {
        return static_cast<std::uint32_t>(Config::max_orders) - free_count_;
    }

    constexpr bool full() const noexcept {
        return free_head_ == npos;
    }

    constexpr void clear() noexcept {
        for (std::size_t i = 0; i < Config::max_orders; ++i) {
            nodes_[i].next_free = (i + 1 < Config::max_orders) ? static_cast<std::uint32_t>(i + 1) : npos;
            nodes_[i].state = 0;
        }
        free_head_ = 0;
        free_count_ = static_cast<std::uint32_t>(Config::max_orders);
    }

    constexpr std::array<Node, Config::max_orders>& nodes() noexcept {
        return nodes_;
    }

    constexpr const std::array<Node, Config::max_orders>& nodes() const noexcept {
        return nodes_;
    }
};

}