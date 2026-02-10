#include <benchmark/benchmark.h>
#include "workloads.hpp"
#include "../../include/runtime/order_book.hpp"
#include "../../include/runtime/config.hpp"

using namespace lob::runtime;
using namespace lob::spec;
using namespace lob::benchmarks;

template <class Config>
struct TradeSink {
    std::vector<Trade> trades;
    void on_trade(const Trade& t) noexcept { trades.push_back(t); }
    void clear() noexcept { trades.clear(); }
};

static void BM_Cancel_Head(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with orders at same price
    for (std::size_t i = 0; i < num_orders; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
    }
    sink.clear();

    std::size_t next_id = num_orders + 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.cancel(1));  // Cancel head
        book.submit({static_cast<OrderId>(next_id++), Side::Sell, 100, 10}, sink);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Cancel_Head)->Args({10})->Args({100})->Args({1000})->Args({10000});

static void BM_Cancel_Tail(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with orders at same price
    for (std::size_t i = 0; i < num_orders; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
    }
    sink.clear();

    std::size_t next_id = num_orders + 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.cancel(static_cast<OrderId>(num_orders)));  // Cancel tail
        book.submit({static_cast<OrderId>(next_id++), Side::Sell, 100, 10}, sink);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Cancel_Tail)->Args({10})->Args({100})->Args({1000})->Args({10000});

static void BM_Cancel_Middle(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with orders at same price
    for (std::size_t i = 0; i < num_orders; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
    }
    sink.clear();

    std::size_t next_id = num_orders + 1;
    OrderId middle_id = static_cast<OrderId>(num_orders / 2);

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.cancel(middle_id));  // Cancel middle
        book.submit({static_cast<OrderId>(next_id++), Side::Sell, 100, 10}, sink);
        // Re-establish middle for next iteration
        // The new order goes to tail, so middle shifts
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Cancel_Middle)->Args({10})->Args({100})->Args({1000})->Args({10000});

static void BM_Cancel_Miss(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Add one order
    book.submit({1, Side::Sell, 100, 10}, sink);
    sink.clear();

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.cancel(999));  // Cancel non-existent
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Cancel_Miss);

static void BM_Cancel_AfterPartialFill(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with large orders
    for (std::size_t i = 0; i < num_orders; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 100}, sink);
    }
    sink.clear();

    std::size_t next_id = num_orders + 1;

    for (auto _ : state) {
        // Partially fill the first order
        book.submit({static_cast<OrderId>(next_id++), Side::Buy, 100, 50}, sink);
        // Cancel the partially filled order
        benchmark::DoNotOptimize(book.cancel(1));
        // Re-add a large order at head
        book.submit({static_cast<OrderId>(next_id++), Side::Sell, 100, 100}, sink);
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Cancel_AfterPartialFill)->Args({10})->Args({100})->Args({1000})->Args({10000});

static void BM_Cancel_DifferentPrices(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with orders at different prices
    for (std::size_t i = 0; i < num_orders; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, static_cast<Price>(100 + i), 10}, sink);
    }
    sink.clear();

    std::size_t next_id = num_orders + 1;

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.cancel(1));  // Cancel at price 100
        book.submit({static_cast<OrderId>(next_id++), Side::Sell, 100, 10}, sink);
    }
    state.SetItemsProcessed(state.iterations());
}
BENCHMARK(BM_Cancel_DifferentPrices)->Args({10})->Args({100})->Args({1000})->Args({10000});