#pragma once

#include "../lob/spec.hpp"
#include "../lob/rules.hpp"
#include <concepts>
#include <optional>

namespace lob::compile_time {

template <class... Ts>
struct TypeList;

template <class List>
concept ValidTypeList = requires {
    List::size;
};

template <spec::Side SideV, spec::Price PriceV, spec::Quantity QuantityV, spec::OrderId IdV>
struct Order;

template <class OrderT>
concept ValidOrder = requires {
    OrderT::side;
    OrderT::price;
    OrderT::quantity;
    OrderT::id;
    requires OrderT::quantity > 0;
};

template <spec::Price PriceV, class... Orders>
struct PriceLevel;

template <class Level>
concept ValidPriceLevel = requires {
    Level::price;
    typename Level::orders;
    Level::order_count;
    Level::total_quantity;
    Level::is_empty;
};

template <class... Levels>
struct SideBook;

template <class Book>
concept ValidSideBook = requires {
    typename Book::levels;
    Book::level_count;
    Book::is_empty;
    typename Book::best_level;
    Book::best_price;
};

template <class Bids, class Asks>
struct Book;

template <class BookT>
concept ValidBook = requires {
    typename BookT::bids;
    typename BookT::asks;
    { BookT::valid } -> std::convertible_to<bool>;
    { BookT::best_bid() } -> std::convertible_to<std::optional<spec::Price>>;
    { BookT::best_ask() } -> std::convertible_to<std::optional<spec::Price>>;
};

template <class BookT>
concept ValidStateBook = ValidBook<BookT> && BookT::valid;

template <ValidStateBook BookT, ValidOrder IncomingOrder>
struct Match;

template <ValidBook NewBook, class TradeList, class Remainder>
struct MatchResult;

template <ValidBook BookT>
struct RuntimeBookView;

}