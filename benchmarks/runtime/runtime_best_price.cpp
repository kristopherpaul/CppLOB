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

static void BM_BestPrice_EmptyBook(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.best_bid());
        benchmark::DoNotOptimize(book.best_ask());
    }
    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_BestPrice_EmptyBook);

static void BM_BestPrice_SingleLevel(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    book.submit({1, Side::Buy, 100, 10}, sink);
    book.submit({2, Side::Sell, 101, 10}, sink);
    sink.clear();

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.best_bid());
        benchmark::DoNotOptimize(book.best_ask());
    }
    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_BestPrice_SingleLevel);

static void BM_BestPrice_SparseBitmap(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Add orders at sparse price levels (every 100th price)
    for (int p = 100; p <= 10000; p += 100) {
        book.submit({static_cast<OrderId>(p * 2), Side::Buy, static_cast<Price>(p), 10}, sink);
        book.submit({static_cast<OrderId>(p * 2 + 1), Side::Sell, static_cast<Price>(p + 1), 10}, sink);
    }
    sink.clear();

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.best_bid());
        benchmark::DoNotOptimize(book.best_ask());
    }
    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_BestPrice_SparseBitmap);

static void BM_BestPrice_DenseBitmap(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Fill all price levels
    for (int p = 1; p <= 10000; ++p) {
        book.submit({static_cast<OrderId>(p * 2), Side::Buy, static_cast<Price>(p), 10}, sink);
        book.submit({static_cast<OrderId>(p * 2 + 1), Side::Sell, static_cast<Price>(p), 10}, sink);
    }
    sink.clear();

    for (auto _ : state) {
        benchmark::DoNotOptimize(book.best_bid());
        benchmark::DoNotOptimize(book.best_ask());
    }
    state.SetItemsProcessed(state.iterations() * 2);
}
BENCHMARK(BM_BestPrice_DenseBitmap);

static void BM_BestPrice_AfterEachSubmit(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_no_cross_workload(num_orders, 42, 1, 10000);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    for (auto _ : state) {
        for (const auto& order : workload.submits) {
            benchmark::DoNotOptimize(book.submit(order, sink));
            benchmark::DoNotOptimize(book.best_bid());
            benchmark::DoNotOptimize(book.best_ask());
        }
        book.clear();
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * workload.submits.size() * 3);
}
BENCHMARK(BM_BestPrice_AfterEachSubmit)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_BestPrice_AfterEachCancel(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-populate
    for (std::size_t i = 0; i < num_orders; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
    }
    sink.clear();

    for (auto _ : state) {
        for (std::size_t i = 0; i < num_orders; ++i) {
            benchmark::DoNotOptimize(book.cancel(static_cast<OrderId>(i + 1)));
            benchmark::DoNotOptimize(book.best_bid());
            benchmark::DoNotOptimize(book.best_ask());
        }
        // Re-populate
        for (std::size_t i = 0; i < num_orders; ++i) {
            book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
        }
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * num_orders * 3);
}
BENCHMARK(BM_BestPrice_AfterEachCancel)->Args({1000})->Args({10000})->Args({50000})->Args({100000});