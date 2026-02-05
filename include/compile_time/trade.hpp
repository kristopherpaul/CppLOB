#pragma once

#include "../lob/spec.hpp"

namespace lob::compile_time {

template <spec::OrderId AggressorV, spec::OrderId RestingV, spec::Price PriceV, spec::Quantity QuantityV>
struct Trade {
    static constexpr spec::OrderId aggressor = AggressorV;
    static constexpr spec::OrderId resting = RestingV;
    static constexpr spec::Price price = PriceV;
    static constexpr spec::Quantity quantity = QuantityV;
};

template <class TradeT>
constexpr spec::Trade to_runtime_trade() noexcept {
    return spec::Trade{
        TradeT::aggressor,
        TradeT::resting,
        TradeT::price,
        TradeT::quantity
    };
}

}