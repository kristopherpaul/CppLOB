#include "runner.hpp"
#include "reference/reference_book.hpp"
#include "runtime/order_book.hpp"

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

    return reference_passed && runtime_passed ? 0 : 1;
}