#include <benchmark/benchmark.h>
#include "workloads.hpp"
#include "evolution/stage0_reference.hpp"
#include "evolution/stage1_flat_ladder.hpp"
#include "evolution/stage2_intrusive_fifo.hpp"
#include "evolution/stage3_fixed_pool.hpp"
#include "evolution/stage4_order_index.hpp"
#include "evolution/stage5_hierarchical_bitmap.hpp"
#include "evolution/stage6_side_specialized.hpp"

using namespace lob::evolution;
using namespace lob::benchmarks;
using namespace lob::spec;

template <class Book>
struct TradeSink {
    std::vector<Trade> trades;
    void on_trade(const Trade& t) noexcept { trades.push_back(t); }
    void clear() noexcept { trades.clear(); }
};

template <class Book>
void run_submit_benchmark(benchmark::State& state, const Workload& workload) {
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

template <class Book>
void run_cancel_benchmark(benchmark::State& state, const Workload& workload) {
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with all submits
    for (const auto& order : workload.submits) {
        book.submit(order, sink);
    }
    sink.clear();

    std::size_t cancel_idx = 0;
    for (auto _ : state) {
        if (cancel_idx < workload.cancels.size()) {
            benchmark::DoNotOptimize(book.cancel(workload.cancels[cancel_idx++]));
        } else {
            cancel_idx = 0;
            benchmark::DoNotOptimize(book.cancel(workload.cancels[0]));
        }
    }
    state.SetItemsProcessed(state.iterations());
}

template <class Book>
void run_best_price_benchmark(benchmark::State& state) {
    Book book;
    TradeSink<Book> sink;

    // Pre-populate with dense book
    for (int p = 1; p <= 1000; ++p) {
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

using Book0 = Stage0_ReferenceBook<1, 10000, 100000>;
using Book1 = Stage1_FlatLadder<1, 10000, 100000>;
using Book2 = Stage2_IntrusiveFIFO<1, 10000, 100000>;
using Book3 = Stage3_FixedPool<1, 10000, 100000>;
using Book4 = Stage4_OrderIndex<1, 10000, 100000>;
using Book5 = Stage5_HierarchicalBitmap<1, 10000, 100000>;
using Book6 = Stage6_SideSpecialized<1, 10000, 100000>;

static void BM_Evolution_Submit_NoCross_Stage0(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book0>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage0)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_NoCross_Stage1(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book1>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage1)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_NoCross_Stage2(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book2>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage2)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_NoCross_Stage3(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book3>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage3)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_NoCross_Stage4(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book4>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage4)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_NoCross_Stage5(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book5>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage5)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_NoCross_Stage6(benchmark::State& state) {
    auto workload = generate_no_cross_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book6>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_NoCross_Stage6)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage0(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book0>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage0)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage1(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book1>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage1)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage2(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book2>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage2)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage3(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book3>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage3)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage4(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book4>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage4)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage5(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book5>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage5)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Submit_Crossing_Stage6(benchmark::State& state) {
    auto workload = generate_crossing_workload(state.range(0), 42, 1, 10000);
    run_submit_benchmark<Book6>(state, workload);
}
BENCHMARK(BM_Evolution_Submit_Crossing_Stage6)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage0(benchmark::State& state) {
    auto workload = generate_evolution_workload(0, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book0>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage0)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage1(benchmark::State& state) {
    auto workload = generate_evolution_workload(1, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book1>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage1)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage2(benchmark::State& state) {
    auto workload = generate_evolution_workload(2, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book2>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage2)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage3(benchmark::State& state) {
    auto workload = generate_evolution_workload(3, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book3>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage3)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage4(benchmark::State& state) {
    auto workload = generate_evolution_workload(4, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book4>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage4)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage5(benchmark::State& state) {
    auto workload = generate_evolution_workload(5, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book5>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage5)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_Cancel_Stage6(benchmark::State& state) {
    auto workload = generate_evolution_workload(6, state.range(0), 42, 1, 10000);
    run_cancel_benchmark<Book6>(state, workload);
}
BENCHMARK(BM_Evolution_Cancel_Stage6)->Args({1000})->Args({10000})->Args({50000})->Args({100000});

static void BM_Evolution_BestPrice_Stage0(benchmark::State& state) {
    run_best_price_benchmark<Book0>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage0);

static void BM_Evolution_BestPrice_Stage1(benchmark::State& state) {
    run_best_price_benchmark<Book1>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage1);

static void BM_Evolution_BestPrice_Stage2(benchmark::State& state) {
    run_best_price_benchmark<Book2>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage2);

static void BM_Evolution_BestPrice_Stage3(benchmark::State& state) {
    run_best_price_benchmark<Book3>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage3);

static void BM_Evolution_BestPrice_Stage4(benchmark::State& state) {
    run_best_price_benchmark<Book4>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage4);

static void BM_Evolution_BestPrice_Stage5(benchmark::State& state) {
    run_best_price_benchmark<Book5>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage5);

static void BM_Evolution_BestPrice_Stage6(benchmark::State& state) {
    run_best_price_benchmark<Book6>(state);
}
BENCHMARK(BM_Evolution_BestPrice_Stage6);