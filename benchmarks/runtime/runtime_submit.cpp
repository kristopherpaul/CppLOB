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

static void BM_Submit_NoCross(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_no_cross_workload(num_orders, 42, 1, 10000);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    for (auto _ : state) {
        for (const auto& order : workload.submits) {
            benchmark::DoNotOptimize(book.submit(order, sink));
        }
        book.clear();
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * workload.submits.size());
}
BENCHMARK(BM_Submit_NoCross)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Submit_Crossing(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_crossing_workload(num_orders, 42, 1, 10000);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    for (auto _ : state) {
        for (const auto& order : workload.submits) {
            benchmark::DoNotOptimize(book.submit(order, sink));
        }
        book.clear();
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * workload.submits.size());
}
BENCHMARK(BM_Submit_Crossing)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Submit_MixedCrossRatio_25(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_workload(num_orders, 0.0, 42, 1, 10000);
    
    // Modify to have 25% crossing
    std::size_t cross_count = num_orders / 4;
    for (std::size_t i = 0; i < cross_count && i < workload.submits.size(); ++i) {
        if (workload.submits[i].side == Side::Buy) {
            workload.submits[i].price = 10000;
        } else {
            workload.submits[i].price = 1;
        }
    }
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    for (auto _ : state) {
        for (const auto& order : workload.submits) {
            benchmark::DoNotOptimize(book.submit(order, sink));
        }
        book.clear();
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * workload.submits.size());
}
BENCHMARK(BM_Submit_MixedCrossRatio_25)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Submit_MixedCrossRatio_50(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_workload(num_orders, 0.0, 42, 1, 10000);
    
    // Modify to have 50% crossing
    std::size_t cross_count = num_orders / 2;
    for (std::size_t i = 0; i < cross_count && i < workload.submits.size(); ++i) {
        if (workload.submits[i].side == Side::Buy) {
            workload.submits[i].price = 10000;
        } else {
            workload.submits[i].price = 1;
        }
    }
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    for (auto _ : state) {
        for (const auto& order : workload.submits) {
            benchmark::DoNotOptimize(book.submit(order, sink));
        }
        book.clear();
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * workload.submits.size());
}
BENCHMARK(BM_Submit_MixedCrossRatio_50)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Submit_MixedCrossRatio_75(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_workload(num_orders, 0.0, 42, 1, 10000);
    
    // Modify to have 75% crossing
    std::size_t cross_count = (num_orders * 3) / 4;
    for (std::size_t i = 0; i < cross_count && i < workload.submits.size(); ++i) {
        if (workload.submits[i].side == Side::Buy) {
            workload.submits[i].price = 10000;
        } else {
            workload.submits[i].price = 1;
        }
    }
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    for (auto _ : state) {
        for (const auto& order : workload.submits) {
            benchmark::DoNotOptimize(book.submit(order, sink));
        }
        book.clear();
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * workload.submits.size());
}
BENCHMARK(BM_Submit_MixedCrossRatio_75)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Submit_PreFilled_50Percent(benchmark::State& state) {
    const std::size_t num_orders = state.range(0);
    auto workload = generate_no_cross_workload(num_orders, 42, 1, 10000);
    
    using Book = OrderBookT<1, 10000, 100000>;
    Book book;
    TradeSink<Book> sink;

    // Pre-fill 50%
    std::size_t prefill_count = workload.submits.size() / 2;
    for (std::size_t i = 0; i < prefill_count; ++i) {
        book.submit(workload.submits[i], sink);
    }
    sink.clear();

    for (auto _ : state) {
        for (std::size_t i = prefill_count; i < workload.submits.size(); ++i) {
            benchmark::DoNotOptimize(book.submit(workload.submits[i], sink));
        }
        // Reset to pre-filled state
        book.clear();
        for (std::size_t i = 0; i < prefill_count; ++i) {
            book.submit(workload.submits[i], sink);
        }
        sink.clear();
    }
    state.SetItemsProcessed(state.iterations() * (workload.submits.size() - prefill_count));
}
BENCHMARK(BM_Submit_PreFilled_50Percent)->Args({1000})->Args({10000})->Args({50000})->Args({100000});