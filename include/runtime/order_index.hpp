#pragma once

#include "config.hpp"
#include "../lob/spec.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace lob::runtime {

template <class Config>
class OrderIndex {
    struct Entry {
        spec::OrderId id = 0;
        std::uint32_t pool_index = npos;
        std::uint8_t state = 0;
    };

    static constexpr std::size_t capacity = []() consteval {
        std::size_t cap = Config::max_orders * 2;
        cap |= cap >> 1;
        cap |= cap >> 2;
        cap |= cap >> 4;
        cap |= cap >> 8;
        cap |= cap >> 16;
        cap |= cap >> 32;
        return cap + 1;
    }();

    static constexpr std::uint8_t empty_state = 0;
    static constexpr std::uint8_t occupied_state = 1;
    static constexpr std::uint8_t tombstone_state = 2;

    std::array<Entry, capacity> entries_;
    std::size_t size_ = 0;

    constexpr std::size_t hash(spec::OrderId id) const noexcept {
        return static_cast<std::size_t>(id * 11400714819323198485ull) & (capacity - 1);
    }

    constexpr std::size_t find_slot(spec::OrderId id) const noexcept {
        std::size_t slot = hash(id);
        std::size_t probes = 0;
        
        while (probes < capacity) {
            auto entry_id = entries_[slot].id;
            if (entries_[slot].state == empty_state) {
                return slot;
            }
            if (entries_[slot].state == occupied_state && entry_id == id) return slot;
            slot = (slot + 1) & (capacity - 1);
            ++probes;
        }
        return capacity;
    }

    constexpr std::size_t find_slot_for_insert(spec::OrderId id) const noexcept {
        std::size_t slot = hash(id);
        std::size_t first_tombstone = capacity;
        std::size_t probes = 0;

        while (probes < capacity) {
            const auto entry_id = entries_[slot].id;
            if (entries_[slot].state == empty_state) {
                return first_tombstone == capacity ? slot : first_tombstone;
            }
            if (entries_[slot].state == occupied_state && entry_id == id) {
                return slot;
            }
            if (entries_[slot].state == tombstone_state && first_tombstone == capacity) {
                first_tombstone = slot;
            }
            slot = (slot + 1) & (capacity - 1);
            ++probes;
        }
        return first_tombstone;
    }

    constexpr std::size_t find_slot_for_erase(spec::OrderId id) const noexcept {
        std::size_t slot = hash(id);
        std::size_t probes = 0;
        
        while (probes < capacity) {
            auto entry_id = entries_[slot].id;
            if (entries_[slot].state == empty_state) {
                return capacity;
            }
            if (entries_[slot].state == occupied_state && entry_id == id) {
                return slot;
            }
            slot = (slot + 1) & (capacity - 1);
            ++probes;
        }
        return capacity;
    }

public:
    constexpr OrderIndex() noexcept = default;

    constexpr std::uint32_t find(spec::OrderId id) const noexcept {
        std::size_t slot = find_slot(id);
        if (slot == capacity) return npos;
        return entries_[slot].pool_index;
    }

    constexpr bool insert(spec::OrderId id, std::uint32_t pool_index) noexcept {
        if (size_ >= capacity / 2) return false;
        
        std::size_t slot = find_slot_for_insert(id);
        if (slot == capacity) return false;
        
        if (entries_[slot].state == occupied_state && entries_[slot].id == id) return false;
        
        entries_[slot].id = id;
        entries_[slot].pool_index = pool_index;
        entries_[slot].state = occupied_state;
        ++size_;
        return true;
    }

    constexpr bool erase(spec::OrderId id) noexcept {
        std::size_t slot = find_slot_for_erase(id);
        if (slot == capacity) return false;
        
        entries_[slot].id = 0;
        entries_[slot].pool_index = npos;
        entries_[slot].state = tombstone_state;
        --size_;
        return true;
    }

    constexpr void clear() noexcept {
        for (auto& entry : entries_) {
            entry.id = 0;
            entry.pool_index = npos;
            entry.state = empty_state;
        }
        size_ = 0;
    }

    constexpr std::size_t size() const noexcept {
        return size_;
    }

    constexpr bool empty() const noexcept {
        return size_ == 0;
    }
};

}