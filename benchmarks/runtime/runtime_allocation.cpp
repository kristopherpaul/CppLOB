#include <benchmark/benchmark.h>
#include "workloads.hpp"
#include "../../include/runtime/order_book.hpp"
#include "../../include/runtime/config.hpp"

using namespace lob::runtime;
using namespace lob::spec;
using namespace lob::benchmarks;

struct AllocationTracker {
    static inline std::atomic<std::size_t> alloc_count{0};
    static inline std::atomic<std::size_t> dealloc_count{0};

    static void reset() noexcept {
        alloc_count.store(0, std::memory_order_relaxed);
        dealloc_count.store(0, std::memory_order_relaxed);
    }

    static void* allocate(std::size_t n) noexcept {
        ++alloc_count;
        return ::operator new(n);
    }

    static void deallocate(void* p) noexcept {
        ++dealloc_count;
        ::operator delete(p);
    }
};

template <class Config>
struct TrackingTradeSink {
    std::vector<Trade> trades;
    void on_trade(const Trade& t) noexcept { trades.push_back(t); }
    void clear() noexcept { trades.clear(); }
};

static void BM_Allocation_Count_Init(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    
    for (auto _ : state) {
        AllocationTracker::reset();
        {
            Book book;
            TrackingTradeSink<Book> sink;
            
            // Pre-fill
            for (int i = 0; i < 1000; ++i) {
                book.submit({static_cast<OrderId>(i + 1), Side::Buy, 100, 10}, sink);
            }
            
            auto init_allocs = AllocationTracker::alloc_count.load();
            auto init_deallocs = AllocationTracker::dealloc_count.load();
            benchmark::DoNotOptimize(init_allocs);
            benchmark::DoNotOptimize(init_deallocs);
        }
    }
    state.counters["Init_Allocs"] = benchmark::Counter(0, benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Allocation_Count_Init);

static void BM_Allocation_Count_HotPath_Submit(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    
    Book book;
    TrackingTradeSink<Book> sink;
    
    // Pre-fill
    for (int i = 0; i < 1000; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Buy, 100, 10}, sink);
    }
    sink.clear();
    
    AllocationTracker::reset();

    for (auto _ : state) {
        for (int i = 1000; i < 2000; ++i) {
            benchmark::DoNotOptimize(book.submit({static_cast<OrderId>(i), Side::Buy, 100, 10}, sink));
        }
        book.clear();
        sink.clear();
        AllocationTracker::reset();
    }
    
    auto allocs = AllocationTracker::alloc_count.load();
    auto deallocs = AllocationTracker::dealloc_count.load();
    state.counters["Hot_Allocs"] = benchmark::Counter(allocs, benchmark::Counter::kIsRate);
    state.counters["Hot_Deallocs"] = benchmark::Counter(deallocs, benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Allocation_Count_HotPath_Submit);

static void BM_Allocation_Count_HotPath_Cancel(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    
    Book book;
    TrackingTradeSink<Book> sink;
    
    // Pre-fill
    for (int i = 0; i < 1000; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Buy, 100, 10}, sink);
    }
    sink.clear();
    
    AllocationTracker::reset();

    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            benchmark::DoNotOptimize(book.cancel(static_cast<OrderId>(i + 1)));
        }
        // Re-fill
        for (int i = 0; i < 1000; ++i) {
            book.submit({static_cast<OrderId>(i + 1), Side::Buy, 100, 10}, sink);
        }
        sink.clear();
        AllocationTracker::reset();
    }
    
    auto allocs = AllocationTracker::alloc_count.load();
    auto deallocs = AllocationTracker::dealloc_count.load();
    state.counters["Hot_Allocs"] = benchmark::Counter(allocs, benchmark::Counter::kIsRate);
    state.counters["Hot_Deallocs"] = benchmark::Counter(deallocs, benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Allocation_Count_HotPath_Cancel);

static void BM_Allocation_Count_HotPath_Match(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    
    Book book;
    TrackingTradeSink<Book> sink;
    
    // Pre-fill asks
    for (int i = 0; i < 1000; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
    }
    sink.clear();
    
    AllocationTracker::reset();

    for (auto _ : state) {
        // Submit crossing buys
        for (int i = 1000; i < 2000; ++i) {
            benchmark::DoNotOptimize(book.submit({static_cast<OrderId>(i), Side::Buy, 100, 10}, sink));
        }
        book.clear();
        // Re-fill asks
        for (int i = 0; i < 1000; ++i) {
            book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
        }
        sink.clear();
        AllocationTracker::reset();
    }
    
    auto allocs = AllocationTracker::alloc_count.load();
    auto deallocs = AllocationTracker::dealloc_count.load();
    state.counters["Hot_Allocs"] = benchmark::Counter(allocs, benchmark::Counter::kIsRate);
    state.counters["Hot_Deallocs"] = benchmark::Counter(deallocs, benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Allocation_Count_HotPath_Match);

static void BM_Allocation_Zero_Target(benchmark::State& state) {
    using Book = OrderBookT<1, 10000, 100000>;
    
    Book book;
    TrackingTradeSink<Book> sink;
    
    // Pre-fill
    for (int i = 0; i < 1000; ++i) {
        book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
    }
    sink.clear();
    
    AllocationTracker::reset();

    for (auto _ : state) {
        // Mix of operations
        book.submit({2000, Side::Buy, 100, 5}, sink);
        book.cancel(1);
        book.submit({2001, Side::Buy, 100, 5}, sink);
        book.cancel(2);
        benchmark::DoNotOptimize(book.best_bid());
        benchmark::DoNotOptimize(book.best_ask());
        
        book.clear();
        for (int i = 0; i < 1000; ++i) {
            book.submit({static_cast<OrderId>(i + 1), Side::Sell, 100, 10}, sink);
        }
        sink.clear();
        AllocationTracker::reset();
    }
    
    auto allocs = AllocationTracker::alloc_count.load();
    auto deallocs = AllocationTracker::dealloc_count.load();
    state.counters["Hot_Allocs"] = benchmark::Counter(allocs, benchmark::Counter::kIsRate);
    state.counters["Hot_Deallocs"] = benchmark::Counter(deallocs, benchmark::Counter::kIsRate);
}
BENCHMARK(BM_Allocation_Zero_Target);