#pragma once

#include "spec.hpp"
#include <concepts>

namespace lob::spec {

template <class Sink>
concept TradeSink = requires(Sink s, const Trade& t) {
    { s.on_trade(t) } -> std::same_as<void>;
};

template <class Book>
concept RuntimeOrderBook = requires(Book b, OrderInput o) {
    { b.submit(o) } -> std::same_as<bool>;
    { b.cancel(o.id) } -> std::same_as<bool>;
    { b.best_bid() } -> std::same_as<std::optional<Price>>;
    { b.best_ask() } -> std::same_as<std::optional<Price>>;
    { b.snapshot() };
};

template <class Book>
concept RuntimeOrderBookWithSink = RuntimeOrderBook<Book> && requires(Book b, OrderInput o) {
    requires TradeSink<decltype(b)>;
    { b.submit(o, b) } -> std::same_as<bool>;
};

template <class BookT>
concept CompileTimeBook = requires {
    typename BookT::bids;
    typename BookT::asks;
    { BookT::valid } -> std::convertible_to<bool>;
    { BookT::best_bid() } -> std::convertible_to<std::optional<Price>>;
    { BookT::best_ask() } -> std::convertible_to<std::optional<Price>>;
};

}