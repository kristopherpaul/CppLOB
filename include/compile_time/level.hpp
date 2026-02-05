#pragma once

#include "order.hpp"
#include "type_list.hpp"
#include <type_traits>

namespace lob::compile_time {

template <class List, std::size_t N>
struct PopFrontN;

template <class... Ts>
struct PopFrontN<TypeList<Ts...>, 0> {
    using type = TypeList<Ts...>;
};

template <class T, class... Ts>
struct PopFrontN<TypeList<T, Ts...>, 1> {
    using type = TypeList<Ts...>;
};

template <class T, class... Ts, std::size_t N>
struct PopFrontN<TypeList<T, Ts...>, N> {
    using type = typename PopFrontN<TypeList<Ts...>, N - 1>::type;
};

// Helper to convert TypeList back to PriceLevel
template <spec::Price PriceV, class List>
struct MakePriceLevel;

template <spec::Price PriceV, class... Orders>
struct MakePriceLevel<PriceV, TypeList<Orders...>> {
    using type = PriceLevel<PriceV, Orders...>;
};

// Helper to compute total quantity
template <class List>
struct TotalQuantity;

template <>
struct TotalQuantity<TypeList<>> {
    static constexpr spec::Quantity value = 0;
};

template <class Order, class... Orders>
struct TotalQuantity<TypeList<Order, Orders...>> {
    static constexpr spec::Quantity value = Order::quantity + TotalQuantity<TypeList<Orders...>>::value;
};

// Helper to get best order
template <bool Empty, class List>
struct BestOrderImpl;

template <class List>
struct BestOrderImpl<true, List> {
    using type = void;
};

template <class Order, class... Orders>
struct BestOrderImpl<false, TypeList<Order, Orders...>> {
    using type = Order;
};

template <spec::Price PriceV, class... Orders>
struct PriceLevel {
    static constexpr spec::Price price = PriceV;
    using orders = TypeList<Orders...>;
    static constexpr std::size_t order_count = sizeof...(Orders);

    static constexpr spec::Quantity total_quantity = TotalQuantity<orders>::value;

    static constexpr bool is_empty = (order_count == 0);

    using best_order = typename BestOrderImpl<is_empty, orders>::type;

    template <ValidOrder NewOrder>
    using push_back = PriceLevel<PriceV, Orders..., NewOrder>;

    template <std::size_t N>
    using remove_front_n = typename MakePriceLevel<PriceV, typename PopFrontN<orders, N>::type>::type;
};

template <spec::Price PriceV>
using EmptyPriceLevel = PriceLevel<PriceV>;

template <ValidPriceLevel Level, ValidOrder NewOrder>
using LevelPushBack_t = typename Level::template push_back<NewOrder>;

template <ValidPriceLevel Level, std::size_t N>
using LevelPopFrontN_t = typename Level::template remove_front_n<N>;

template <ValidPriceLevel Level>
using LevelBestOrder_t = typename Level::best_order;

template <ValidPriceLevel Level>
constexpr spec::Quantity level_total_quantity_v = Level::total_quantity;

template <ValidPriceLevel Level>
constexpr std::size_t level_order_count_v = Level::order_count;

template <ValidPriceLevel Level>
constexpr bool level_is_empty_v = Level::is_empty;

}