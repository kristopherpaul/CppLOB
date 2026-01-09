#include "runner.hpp"
#include "../../tests/reference/reference_book.hpp"
#include <memory>

int main() {
    std::cout << "Running conformance tests against ReferenceBook...\n\n";

    lob::conformance::run_all_scenarios([]() {
        return lob::reference::ReferenceBook{};
    });

    return 0;
}