#pragma once

#include "concepts.hpp"
#include <type_traits>

namespace lob::compile_time {

template <class... Ts>
struct TypeList;

template <std::size_t I, class List>
struct At;

template <std::size_t I, class List>
using At_t = typename At<I, List>::type;

template <class... Ts>
struct TypeList {
    static constexpr std::size_t size = sizeof...(Ts);
};

template <class List>
struct ListSize;

template <class... Ts>
struct ListSize<TypeList<Ts...>> : std::integral_constant<std::size_t, sizeof...(Ts)> {};

template <class List>
constexpr std::size_t list_size_v = ListSize<List>::value;

template <class List, class T>
struct PushBack;

template <class T, class List>
struct PushFront;

template <class T, class... Ts>
struct PushFront<T, TypeList<Ts...>> {
    using type = TypeList<T, Ts...>;
};

template <class T, class List>
using PushFront_t = typename PushFront<T, List>::type;

template <class... Ts, class T>
struct PushBack<TypeList<Ts...>, T> {
    using type = TypeList<Ts..., T>;
};

template <class List, class T>
using PushBack_t = typename PushBack<List, T>::type;

template <class List>
struct PopFront;

template <class T, class... Ts>
struct PopFront<TypeList<T, Ts...>> {
    using type = TypeList<Ts...>;
};

template <class List>
using PopFront_t = typename PopFront<List>::type;

template <class List1, class List2>
struct Concat;

template <class... Ts1, class... Ts2>
struct Concat<TypeList<Ts1...>, TypeList<Ts2...>> {
    using type = TypeList<Ts1..., Ts2...>;
};

template <class List1, class List2>
using Concat_t = typename Concat<List1, List2>::type;

template <class List, template <class> class Pred>
struct Filter;

template <template <class> class Pred>
struct Filter<TypeList<>, Pred> {
    using type = TypeList<>;
};

template <class T, class... Ts, template <class> class Pred>
struct Filter<TypeList<T, Ts...>, Pred> {
private:
    using filtered_tail = typename Filter<TypeList<Ts...>, Pred>::type;
public:
    using type = std::conditional_t<
        Pred<T>::value,
        PushFront_t<T, filtered_tail>,
        filtered_tail
    >;
};

template <class List, template <class> class Pred>
using Filter_t = typename Filter<List, Pred>::type;

template <std::size_t I, class T, class... Ts>
struct At<I, TypeList<T, Ts...>> {
    using type = typename At<I - 1, TypeList<Ts...>>::type;
};

template <class T, class... Ts>
struct At<0, TypeList<T, Ts...>> {
    using type = T;
};

template <class List>
struct Front;

template <class T, class... Ts>
struct Front<TypeList<T, Ts...>> {
    using type = T;
};

template <class List>
using Front_t = typename Front<List>::type;

template <class List>
struct Back;

template <class T>
struct Back<TypeList<T>> {
    using type = T;
};

template <class T, class... Ts>
struct Back<TypeList<T, Ts...>> {
    using type = typename Back<TypeList<Ts...>>::type;
};

template <class List>
using Back_t = typename Back<List>::type;

template <class List, class T>
struct Contains;

template <class... Ts, class T>
struct Contains<TypeList<Ts...>, T> : std::bool_constant<(std::is_same_v<Ts, T> || ...)> {};

template <class List, class T>
constexpr bool contains_v = Contains<List, T>::value;

template <class List, template <class> class Pred>
struct FindIf;

template <template <class> class Pred>
struct FindIf<TypeList<>, Pred> {
    static constexpr std::size_t value = static_cast<std::size_t>(-1);
};

template <class T, class... Ts, template <class> class Pred>
struct FindIf<TypeList<T, Ts...>, Pred> {
    static constexpr std::size_t value = Pred<T>::value ? 0 : (FindIf<TypeList<Ts...>, Pred>::value == static_cast<std::size_t>(-1) ? static_cast<std::size_t>(-1) : 1 + FindIf<TypeList<Ts...>, Pred>::value);
};

template <class List, template <class> class Pred>
constexpr std::size_t find_if_v = FindIf<List, Pred>::value;

}