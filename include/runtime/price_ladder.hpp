#pragma once

#include "config.hpp"
#include "price_level.hpp"
#include <array>

namespace lob::runtime {

template <class Config>
class PriceLadder {
    std::array<PriceLevel, Config::num_levels> levels_;

public:
    constexpr PriceLadder() noexcept = default;

    constexpr PriceLevel& level(spec::Price price) noexcept {
        return levels_[Config::price_to_index(price)];
    }

    constexpr const PriceLevel& level(spec::Price price) const noexcept {
        return levels_[Config::price_to_index(price)];
    }

    constexpr PriceLevel& level_by_index(std::size_t index) noexcept {
        return levels_[index];
    }

    constexpr const PriceLevel& level_by_index(std::size_t index) const noexcept {
        return levels_[index];
    }

    constexpr void clear() noexcept {
        for (auto& level : levels_) {
            level.reset();
        }
    }

    constexpr std::size_t size() const noexcept {
        return Config::num_levels;
    }
};

}