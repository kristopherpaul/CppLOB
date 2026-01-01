#pragma once

#include <cstdint>
#include <optional>

namespace lob::spec {

enum class Side : std::uint8_t { Buy, Sell };

using Price = std::int32_t;
using Quantity = std::uint64_t;
using OrderId = std::uint64_t;

struct OrderInput {
    OrderId id;
    Side side;
    Price price;
    Quantity quantity;
};

struct Trade {
    OrderId aggressor;
    OrderId resting;
    Price price;
    Quantity quantity;
};

constexpr Side opposite(Side s) noexcept {
    return s == Side::Buy ? Side::Sell : Side::Buy;
}

constexpr bool is_buy(Side s) noexcept {
    return s == Side::Buy;
}

constexpr bool is_sell(Side s) noexcept {
    return s == Side::Sell;
}

}