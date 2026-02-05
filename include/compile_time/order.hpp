#pragma once

#include "concepts.hpp"
#include "../lob/rules.hpp"

namespace lob::compile_time {

template <spec::Side SideV, spec::Price PriceV, spec::Quantity QuantityV, spec::OrderId IdV>
struct Order {
    static constexpr spec::Side side = SideV;
    static constexpr spec::Price price = PriceV;
    static constexpr spec::Quantity quantity = QuantityV;
    static constexpr spec::OrderId id = IdV;

    static constexpr bool is_buy = (SideV == spec::Side::Buy);
    static constexpr bool is_sell = (SideV == spec::Side::Sell);
};

template <ValidOrder OrderT>
constexpr bool is_buy_order_v = OrderT::is_buy;

template <ValidOrder OrderT>
constexpr bool is_sell_order_v = OrderT::is_sell;

template <class OrderT>
constexpr spec::Side order_side = OrderT::side;

template <class OrderT>
constexpr spec::Price order_price = OrderT::price;

template <class OrderT>
constexpr spec::Quantity order_quantity = OrderT::quantity;

template <class OrderT>
constexpr spec::OrderId order_id = OrderT::id;

template <class OrderT>
constexpr spec::Quantity remaining_quantity = OrderT::quantity;

template <ValidOrder OrderT, spec::Quantity FilledQty>
requires (FilledQty <= OrderT::quantity)
struct OrderWithRemaining {
    static constexpr spec::Side side = OrderT::side;
    static constexpr spec::Price price = OrderT::price;
    static constexpr spec::Quantity quantity = OrderT::quantity - FilledQty;
    static constexpr spec::OrderId id = OrderT::id;

    static constexpr bool is_buy = OrderT::is_buy;
    static constexpr bool is_sell = OrderT::is_sell;
    static constexpr bool is_filled = (quantity == 0);
};

template <ValidOrder OrderT, spec::Quantity FilledQty>
requires (FilledQty <= OrderT::quantity)
using OrderWithRemaining_t = OrderWithRemaining<OrderT, FilledQty>;

template <class OrderT>
concept ValidOrderWithRemaining = requires {
    OrderT::side;
    OrderT::price;
    OrderT::quantity;
    OrderT::id;
    requires OrderT::quantity >= 0;
};

template <ValidOrderWithRemaining OrderT>
constexpr bool is_filled_v = (OrderT::quantity == 0);

}