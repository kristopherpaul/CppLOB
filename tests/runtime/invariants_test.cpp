#include "runtime/order_book.hpp"
#include "runtime/order_pool.hpp"
#include "runtime/order_index.hpp"
#include "runtime/bitmap.hpp"
#include "runtime/price_ladder.hpp"
#include "../support/trade_sink.hpp"
#include <iostream>
#include <vector>

using Config = lob::runtime::RuntimeConfig<1, 1000, 10000>;
using Book = lob::runtime::OrderBook<Config>;

using TradeSink = lob::test_support::TradeSink;

bool test_reuse_after_cancellation() {
    Book book;
    TradeSink sink;
    
    for (int i = 1; i <= 100; ++i) {
        book.submit({static_cast<lob::spec::OrderId>(i), lob::spec::Side::Buy, 100, 10}, sink);
    }
    
    for (int i = 1; i <= 50; ++i) {
        book.cancel(i);
    }
    
    for (int i = 101; i <= 150; ++i) {
        book.submit({static_cast<lob::spec::OrderId>(i), lob::spec::Side::Buy, 100, 10}, sink);
    }
    
    return true; // If we get here without bad_alloc, test passes
}

bool test_pool_freelist_invariants() {
    lob::runtime::OrderPool<Config> pool;
    
    // Initially all free
    if (pool.free_count() != 10000) return false;
    if (pool.used_count() != 0) return false;
    
    // Allocate all
    std::vector<std::uint32_t> indices;
    for (int i = 0; i < 10000; ++i) {
        auto idx = pool.allocate();
        if (idx == Config::npos) return false;
        indices.push_back(idx);
    }
    
    if (pool.free_count() != 0) return false;
    if (pool.used_count() != 10000) return false;
    if (!pool.full()) return false;
    
    // Deallocate all
    for (auto idx : indices) {
        pool.deallocate(idx);
    }
    
    if (pool.free_count() != 10000) return false;
    if (pool.used_count() != 0) return false;
    
    return true;
}

bool test_order_index_invariants() {
    lob::runtime::OrderIndex<Config> index;
    
    // Insert
    for (uint64_t i = 1; i <= 1000; ++i) {
        if (!index.insert(i, static_cast<uint32_t>(i - 1))) return false;
    }
    
    if (index.size() != 1000) return false;
    
    // Find all
    for (uint64_t i = 1; i <= 1000; ++i) {
        if (index.find(i) != i - 1) return false;
    }
    
    // Find non-existent
    if (index.find(99999) != Config::npos) return false;
    
    // Erase half
    for (uint64_t i = 1; i <= 500; ++i) {
        if (!index.erase(i)) return false;
    }
    
    if (index.size() != 500) return false;
    
    // Erase non-existent
    if (index.erase(99999)) return false;
    
    // Re-insert erased
    for (uint64_t i = 1; i <= 500; ++i) {
        if (!index.insert(i, static_cast<uint32_t>(i + 1000))) return false;
    }
    
    if (index.size() != 1000) return false;
    
    return true;
}

bool test_order_index_tombstone_reuse() {
    using SmallConfig = lob::runtime::RuntimeConfig<1, 1000, 8>;
    lob::runtime::OrderIndex<SmallConfig> index;

    if (!index.insert(1, 10)) return false;
    if (!index.insert(33, 11)) return false;
    if (!index.insert(65, 12)) return false;
    if (!index.erase(1)) return false;
    if (index.find(33) != 11 || index.find(65) != 12) return false;
    if (!index.insert(97, 13)) return false;
    return index.find(97) == 13 && index.size() == 3;
}

bool test_order_index_zero_id() {
    using SmallConfig = lob::runtime::RuntimeConfig<1, 1000, 8>;
    lob::runtime::OrderIndex<SmallConfig> index;

    if (!index.insert(0, 10)) return false;
    if (index.find(0) != 10) return false;
    if (!index.erase(0)) return false;
    return index.find(0) == SmallConfig::npos;
}

bool test_bitmap_invariants() {
    lob::runtime::BitmapIndex<Config> bitmap;
    
    // Initially empty
    if (!bitmap.empty()) return false;
    if (bitmap.best_buy().has_value()) return false;
    if (bitmap.best_sell().has_value()) return false;
    
    // Set some prices
    bitmap.set(100); // price 101
    bitmap.set(200); // price 201
    bitmap.set(50);  // price 51
    
    if (bitmap.empty()) return false;
    
    // Best buy should be highest (201)
    if (bitmap.best_buy() != 201) return false;
    
    // Best sell should be lowest (51)
    if (bitmap.best_sell() != 51) return false;
    
    // Clear middle
    bitmap.clear_bit(100);
    
    if (bitmap.best_buy() != 201) return false;
    if (bitmap.best_sell() != 51) return false;
    
    // Clear highest
    bitmap.clear_bit(200);
    
    if (bitmap.best_buy() != 51) return false;
    if (bitmap.best_sell() != 51) return false;
    
    // Clear last
    bitmap.clear_bit(50);
    
    if (!bitmap.empty()) return false;
    if (bitmap.best_buy().has_value()) return false;
    if (bitmap.best_sell().has_value()) return false;
    
    return true;
}

bool test_bitmap_large_configuration() {
    using LargeConfig = lob::runtime::RuntimeConfig<1, 5000, 8>;
    lob::runtime::BitmapIndex<LargeConfig> bitmap;

    bitmap.set(0);
    bitmap.set(4096);
    bitmap.set(4999);

    if (bitmap.best_sell() != 1) return false;
    if (bitmap.best_buy() != 5000) return false;

    bitmap.clear_bit(4999);
    return bitmap.best_buy() == 4097;
}

bool test_price_ladder_invariants() {
    lob::runtime::PriceLadder<Config> ladder;
    
    auto& level = ladder.level(100);
    if (!level.empty()) return false;
    if (level.total_quantity != 0) return false;
    
    level.head = 1;
    level.tail = 1;
    level.total_quantity = 100;
    
    if (level.empty()) return false;
    if (level.total_quantity != 100) return false;
    
    ladder.clear();
    
    if (!level.empty()) return false;
    if (level.total_quantity != 0) return false;
    
    return true;
}

bool test_full_book_invariants() {
    Book book;
    TradeSink sink;
    
    // Add some orders
    book.submit({1, lob::spec::Side::Buy, 100, 10}, sink);
    book.submit({2, lob::spec::Side::Buy, 99, 20}, sink);
    book.submit({3, lob::spec::Side::Sell, 101, 15}, sink);
    book.submit({4, lob::spec::Side::Sell, 102, 25}, sink);
    
    // Check invariants
    if (book.best_bid() != 100) return false;
    if (book.best_ask() != 101) return false;
    if (book.active_order_count() != 4) return false;
    
    // Cross and trade
    book.submit({5, lob::spec::Side::Buy, 102, 30}, sink);
    
    // Should have trades
    if (sink.trades.empty()) return false;
    
    // Best prices should be updated
    if (book.best_bid() != 100) return false; // Still have bid at 100
    if (book.best_ask() != 102) return false; // Ask at 102 partially filled
    
    return true;
}

bool test_fifo_ordering() {
    Book book;
    TradeSink sink;
    
    // Add multiple orders at same price
    book.submit({1, lob::spec::Side::Sell, 100, 10}, sink);
    book.submit({2, lob::spec::Side::Sell, 100, 10}, sink);
    book.submit({3, lob::spec::Side::Sell, 100, 10}, sink);
    
    // Match against them
    book.submit({4, lob::spec::Side::Buy, 100, 15}, sink);
    
    // Should match in FIFO order: 1 then 2
    if (sink.trades.size() != 2) return false;
    if (sink.trades[0].resting != 1) return false;
    if (sink.trades[1].resting != 2) return false;
    if (sink.trades[0].quantity != 10) return false;
    if (sink.trades[1].quantity != 5) return false;
    
    // Remaining: order 2 (5 qty) and order 3 (10 qty) = 2 orders
    if (book.active_order_count() != 2) return false;
    
    // Cancel both remaining
    book.cancel(2);
    book.cancel(3);
    
    if (book.active_order_count() != 0) return false;
    
    return true;
}

int main() {
    std::cout << "Running Runtime Invariant Tests...\n\n";
    
    int passed = 0, failed = 0;
    
    auto run = [&](const char* name, bool (*fn)()) {
        if (fn()) {
            std::cout << "[PASS] " << name << "\n";
            ++passed;
        } else {
            std::cout << "[FAIL] " << name << "\n";
            ++failed;
        }
    };
    
    run("reuse_after_cancellation", test_reuse_after_cancellation);
    run("pool_freelist_invariants", test_pool_freelist_invariants);
    run("order_index_invariants", test_order_index_invariants);
    run("order_index_tombstone_reuse", test_order_index_tombstone_reuse);
    run("order_index_zero_id", test_order_index_zero_id);
    run("bitmap_invariants", test_bitmap_invariants);
    run("bitmap_large_configuration", test_bitmap_large_configuration);
    run("price_ladder_invariants", test_price_ladder_invariants);
    run("full_book_invariants", test_full_book_invariants);
    run("fifo_ordering", test_fifo_ordering);
    
    std::cout << "\nTotal: " << (passed + failed) << ", Passed: " << passed << ", Failed: " << failed << "\n";
    return failed == 0 ? 0 : 1;
}