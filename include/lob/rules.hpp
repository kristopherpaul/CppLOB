#pragma once

#include "spec.hpp"

namespace lob::spec {

constexpr bool crosses(Side incoming_side, Price incoming_price, Price resting_price) noexcept {
    if (incoming_side == Side::Buy) {
        return incoming_price >= resting_price;
    } else {
        return incoming_price <= resting_price;
    }
}

constexpr bool crosses(Side incoming_side, Price incoming_price, std::optional<Price> resting_price) noexcept {
    return resting_price.has_value() && crosses(incoming_side, incoming_price, *resting_price);
}

constexpr Price execution_price([[maybe_unused]] Side incoming_side, Price resting_price) noexcept {
    return resting_price;
}

inline bool valid_price([[maybe_unused]] Price price) noexcept {
    return true;
}

inline bool valid_quantity(Quantity qty) noexcept {
    return qty > 0;
}

inline bool valid_order_input(const OrderInput& o) noexcept {
    return valid_price(o.price) && valid_quantity(o.quantity);
}

}