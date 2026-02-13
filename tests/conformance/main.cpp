#include "runner.hpp"
#include "reference/reference_book.hpp"
#include "runtime/order_book.hpp"
#include <compile_time/match.hpp>

namespace {

using namespace lob::compile_time;
using namespace lob::spec;

using Asks = AddOrder_t<
    AddOrder_t<EmptyBook, Order<Side::Sell, 100, 10, 1>>,
    Order<Side::Sell, 101, 10, 2>>;
using Result = Submit_t<Asks, Order<Side::Buy, 101, 15, 3>>;
using FirstTrade = At_t<0, typename Result::trades>;
using SecondTrade = At_t<1, typename Result::trades>;

constexpr bool compile_time_conformance() {
    constexpr auto view = Result::runtime_view();
    return Result::book::valid &&
           Result::trade_count == 2 &&
           FirstTrade::aggressor == 3 &&
           FirstTrade::resting == 1 &&
           FirstTrade::price == 100 &&
           FirstTrade::quantity == 10 &&
           SecondTrade::aggressor == 3 &&
           SecondTrade::resting == 2 &&
           SecondTrade::price == 101 &&
           SecondTrade::quantity == 5 &&
           view.bids.level_count == 0 &&
           view.asks.level_count == 1 &&
           view.asks.levels[0].price == 101 &&
           view.asks.levels[0].total_quantity == 5 &&
           view.asks.levels[0].order_count == 1;
}

static_assert(compile_time_conformance());

}

int main() {
    std::cout << "Running conformance tests against ReferenceBook...\n";

    const bool reference_passed = lob::conformance::run_all_scenarios([]() {
        return lob::reference::ReferenceBook{};
    });

    std::cout << "\nRunning conformance tests against Runtime OrderBook...\n";

    using RuntimeBook = lob::runtime::OrderBookT<1, 1000, 10000>;
    const bool runtime_passed = lob::conformance::run_all_scenarios([]() {
        return RuntimeBook{};
    });

    std::cout << "\nRunning conformance tests against Compile-Time Book...\n";
    const bool compile_time_passed = compile_time_conformance();
    std::cout << (compile_time_passed ? "[PASS] representative constexpr scenario\n"
                                      : "[FAIL] representative constexpr scenario\n");

    return reference_passed && runtime_passed && compile_time_passed ? 0 : 1;
}