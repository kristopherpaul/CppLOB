#pragma once

#include "scenarios.hpp"
#include "../../tests/reference/reference_book.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace lob::conformance {

struct TestResult {
    std::string scenario_name;
    bool passed;
    std::string error_message;
};

template <class Book>
TestResult run_scenario(const Scenario& scenario, Book& book) {
    reference::ReferenceBook ref;
    std::vector<spec::Trade> ref_trades;
    std::vector<spec::Trade> book_trades;

    for (const auto& step : scenario.steps) {
        bool ref_result = false;
        bool book_result = false;

        if (step.type == ScenarioStep::Type::Add) {
            ref_result = ref.submit(step.order);
            book_result = book.submit(step.order);
        } else {
            ref_result = ref.cancel(step.order.id);
            book_result = book.cancel(step.order.id);
        }

        if (ref_result != step.expected_success) {
            return {scenario.name, false, "Reference result mismatch at step: expected " + 
                    std::string(step.expected_success ? "success" : "failure")};
        }

        if (book_result != step.expected_success) {
            return {scenario.name, false, "Book result mismatch at step: expected " + 
                    std::string(step.expected_success ? "success" : "failure")};
        }

        if (step.type == ScenarioStep::Type::Add) {
            if (ref_result) {
                auto ref_trades_now = ref.trades();
                if (ref_trades_now.size() > ref_trades.size()) {
                    ref_trades.insert(ref_trades.end(), 
                        ref_trades_now.begin() + ref_trades.size(), ref_trades_now.end());
                }
            }
        }
    }

    auto ref_snap = ref.snapshot();
    auto book_snap = book.snapshot();

    auto ref_best_bid = ref_snap.bids.empty() ? std::optional<spec::Price>{} : std::optional<spec::Price>{ref_snap.bids.begin()->first};
    auto ref_best_ask = ref_snap.asks.empty() ? std::optional<spec::Price>{} : std::optional<spec::Price>{ref_snap.asks.begin()->first};

    if (scenario.final_best_bid != book.best_bid()) {
        return {scenario.name, false, "Best bid mismatch: expected " + 
                (scenario.final_best_bid ? std::to_string(*scenario.final_best_bid) : "nullopt") + 
                ", got " + (book.best_bid() ? std::to_string(*book.best_bid()) : "nullopt")};
    }

    if (scenario.final_best_ask != book.best_ask()) {
        return {scenario.name, false, "Best ask mismatch: expected " + 
                (scenario.final_best_ask ? std::to_string(*scenario.final_best_ask) : "nullopt") + 
                ", got " + (book.best_ask() ? std::to_string(*book.best_ask()) : "nullopt")};
    }

    if (scenario.final_best_bid != ref_best_bid) {
        return {scenario.name, false, "Reference best bid mismatch"};
    }

    if (scenario.final_best_ask != ref_best_ask) {
        return {scenario.name, false, "Reference best ask mismatch"};
    }

    return {scenario.name, true, ""};
}

inline void run_all_scenarios(auto book_factory) {
    auto scenarios = all_scenarios();
    std::size_t passed = 0;
    std::size_t failed = 0;

    for (const auto& scenario : scenarios) {
        auto book = book_factory();
        auto result = run_scenario(scenario, book);
        
        if (result.passed) {
            std::cout << "[PASS] " << scenario.name << "\n";
            ++passed;
        } else {
            std::cout << "[FAIL] " << scenario.name << ": " << result.error_message << "\n";
            ++failed;
        }
    }

    std::cout << "\nTotal: " << (passed + failed) << ", Passed: " << passed << ", Failed: " << failed << "\n";
}

}