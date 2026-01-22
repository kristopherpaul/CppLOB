#pragma once

#include "config.hpp"
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace lob::runtime {

template <class Config>
class BitmapIndex {
    static constexpr std::size_t l0_words = Config::bitmap_l0_words;
    static constexpr std::size_t l1_words = Config::bitmap_l1_words;
    static constexpr std::size_t total_words = Config::bitmap_total_words;

    std::array<std::uint64_t, total_words> words_;

public:
    constexpr BitmapIndex() noexcept {
        clear();
    }

    constexpr void clear() noexcept {
        for (auto& w : words_) w = 0;
    }

    constexpr void set(std::size_t price_index) noexcept {
        std::size_t l0_word = price_index >> 6;
        std::uint64_t l0_bit = std::uint64_t(1) << (price_index & 63);
        
        words_[l0_word] |= l0_bit;
    }

    constexpr void clear_bit(std::size_t price_index) noexcept {
        std::size_t l0_word = price_index >> 6;
        std::uint64_t l0_bit = std::uint64_t(1) << (price_index & 63);
        
        words_[l0_word] &= ~l0_bit;
    }

    constexpr bool test(std::size_t price_index) const noexcept {
        std::size_t l0_word = price_index >> 6;
        std::uint64_t l0_bit = std::uint64_t(1) << (price_index & 63);
        return (words_[l0_word] & l0_bit) != 0;
    }

    template <spec::Side Side>
    constexpr std::optional<spec::Price> best_price() const noexcept {
        if constexpr (Side == spec::Side::Buy) {
            return best_buy();
        } else {
            return best_sell();
        }
    }

    constexpr std::optional<spec::Price> best_buy() const noexcept {
        for (std::size_t i = l0_words; i-- > 0;) {
            const std::uint64_t word = words_[i];
            if (word != 0) {
                const std::size_t bit = 63 - std::countl_zero(word);
                const std::size_t price_index = (i << 6) + bit;
                if (price_index < Config::num_levels) return Config::index_to_price(price_index);
            }
        }
        return std::nullopt;
    }

    constexpr std::optional<spec::Price> best_sell() const noexcept {
        for (std::size_t i = 0; i < l0_words; ++i) {
            const std::uint64_t word = words_[i];
            if (word != 0) {
                const std::size_t bit = std::countr_zero(word);
                const std::size_t price_index = (i << 6) + bit;
                if (price_index < Config::num_levels) return Config::index_to_price(price_index);
            }
        }
        return std::nullopt;
    }

    constexpr bool empty() const noexcept {
        for (std::size_t i = 0; i < l0_words; ++i) {
            if (words_[i] != 0) return false;
        }
        return true;
    }
};

}