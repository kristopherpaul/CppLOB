#pragma once

#include "../lob/spec.hpp"
#include <array>
#include <bit>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <limits>

namespace lob::runtime {

inline constexpr std::uint32_t npos = std::numeric_limits<std::uint32_t>::max();

template <spec::Price MinPrice, spec::Price MaxPrice, std::size_t MaxOrders>
struct RuntimeConfig {
    static_assert(MinPrice < MaxPrice, "MinPrice must be less than MaxPrice");
    static_assert(MaxOrders > 0, "MaxOrders must be positive");
    static_assert(MaxOrders <= (std::numeric_limits<std::uint32_t>::max() / 2), 
                  "MaxOrders too large for 32-bit indices");

    static constexpr spec::Price min_price = MinPrice;
    static constexpr spec::Price max_price = MaxPrice;
    static constexpr std::size_t max_orders = MaxOrders;
    static constexpr std::size_t num_levels = static_cast<std::size_t>(MaxPrice - MinPrice + 1);

    static constexpr std::size_t bitmap_l0_words = (num_levels + 63) / 64;
    
    static constexpr std::size_t bitmap_l1_words = []() consteval {
        std::size_t words = bitmap_l0_words;
        std::size_t total = 0;
        while (words > 1) {
            words = (words + 63) / 64;
            total += words;
        }
        return total;
    }();

    static constexpr std::size_t bitmap_total_words = bitmap_l0_words + bitmap_l1_words;

    static constexpr std::uint32_t npos = std::numeric_limits<std::uint32_t>::max();

    static constexpr spec::Price price_to_index(spec::Price price) noexcept {
        return price - MinPrice;
    }

    static constexpr spec::Price index_to_price(std::size_t index) noexcept {
        return MinPrice + static_cast<spec::Price>(index);
    }

    static constexpr bool valid_price(spec::Price price) noexcept {
        return price >= MinPrice && price <= MaxPrice;
    }
};

template <class Config>
concept ValidRuntimeConfig = requires {
    Config::min_price;
    Config::max_price;
    Config::max_orders;
    Config::num_levels;
    Config::npos;
    { Config::price_to_index(Config::min_price) } -> std::convertible_to<std::size_t>;
    { Config::index_to_price(0) } -> std::convertible_to<spec::Price>;
};

}