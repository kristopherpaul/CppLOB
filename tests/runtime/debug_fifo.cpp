#include "runtime/order_book.hpp"
#include "../support/trade_sink.hpp"
#include <iostream>
#include <vector>

using Config = lob::runtime::RuntimeConfig<1, 1000, 10000>;
using Book = lob::runtime::OrderBook<Config>;

struct DiagnosticTradeSink : lob::test_support::TradeSink {
    void on_trade(const lob::spec::Trade& trade) noexcept {
        std::cout << "  Trade: aggressor=" << trade.aggressor << " resting=" << trade.resting
                  << " price=" << trade.price << " qty=" << trade.quantity << "\n";
        lob::test_support::TradeSink::on_trade(trade);
    }
};

int main() {
    Book book;
    DiagnosticTradeSink sink;
    
    std::cout << "Adding sell orders...\n";
    book.submit({1, lob::spec::Side::Sell, 100, 10}, sink);
    book.submit({2, lob::spec::Side::Sell, 100, 10}, sink);
    book.submit({3, lob::spec::Side::Sell, 100, 10}, sink);
    
    std::cout << "Active orders: " << book.active_order_count() << "\n";
    std::cout << "Best ask: " << (book.best_ask() ? std::to_string(*book.best_ask()) : "none") << "\n";
    
    std::cout << "\nAdding buy order (crossing)...\n";
    book.submit({4, lob::spec::Side::Buy, 100, 15}, sink);
    
    std::cout << "\nTrades: " << sink.trades.size() << "\n";
    for (const auto& t : sink.trades) {
        std::cout << "  Trade: aggressor=" << t.aggressor << " resting=" << t.resting 
                  << " price=" << t.price << " qty=" << t.quantity << "\n";
    }
    
    std::cout << "Active orders after: " << book.active_order_count() << "\n";
    std::cout << "Best ask after: " << (book.best_ask() ? std::to_string(*book.best_ask()) : "none") << "\n";
    
    return 0;
}