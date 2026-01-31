#pragma once

#include "scenarios.hpp"
#include "reference/reference_book.hpp"
#include "../support/trade_sink.hpp"
#include <iostream>
#include <string>
#include <vector>

namespace lob::conformance {

struct TestResult {
    std::string scenario_name;
    bool passed;
    std::string error_message;
};

struct OrderState {
    spec::OrderId id;
    spec::Quantity quantity;
};

struct LevelState {
    spec::Price price;
    std::vector<OrderState> orders;
};

using BookState = std::vector<LevelState>;

inline BookState reference_levels(const reference::ReferenceBook::Snapshot& snapshot,
                                  spec::Side side) {
    BookState result;
    if (side == spec::Side::Buy) {
        for (const auto& [price, orders] : snapshot.bids) {
            LevelState level{price, {}};
            for (const auto& order : orders) {
                level.orders.push_back({order.id, order.quantity});
            }
            result.push_back(std::move(level));
        }
    } else {
        for (const auto& [price, orders] : snapshot.asks) {
            LevelState level{price, {}};
            for (const auto& order : orders) {
                level.orders.push_back({order.id, order.quantity});
            }
            result.push_back(std::move(level));
        }
    }
    return result;
}

template <class BookSnapshot>
BookState runtime_levels(const BookSnapshot& snapshot, spec::Side side) {
    if constexpr (!requires { snapshot.bids.front().order_ids; }) {
        return reference_levels(snapshot, side);
    } else {
        BookState result;
        const auto& levels = side == spec::Side::Buy ? snapshot.bids : snapshot.asks;
        for (const auto& level : levels) {
            LevelState state{level.price, {}};
            for (std::size_t i = 0; i < level.order_ids.size(); ++i) {
                state.orders.push_back(OrderState{level.order_ids[i], level.quantities[i]});
            }
            result.push_back(std::move(state));
        }
        return result;
    }
}

inline bool equal_trades(const std::vector<spec::Trade>& actual,
                         const std::vector<ExpectedTrade>& expected) {
    if (actual.size() != expected.size()) return false;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        const auto& trade = actual[i];
        const auto& wanted = expected[i];
        if (trade.aggressor != wanted.aggressor || trade.resting != wanted.resting ||
            trade.price != wanted.price || trade.quantity != wanted.quantity) {
            return false;
        }
    }
    return true;
}

inline bool equal_levels(const BookState& actual,
                         const std::vector<std::pair<spec::Price, std::vector<spec::OrderInput>>>& expected) {
    if (actual.size() != expected.size()) return false;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (actual[i].price != expected[i].first ||
            actual[i].orders.size() != expected[i].second.size()) {
            return false;
        }
        for (std::size_t j = 0; j < expected[i].second.size(); ++j) {
            if (actual[i].orders[j].id != expected[i].second[j].id ||
                actual[i].orders[j].quantity != expected[i].second[j].quantity) {
                return false;
            }
        }
    }
    return true;
}

template <class Book>
bool submit(Book& book, const spec::OrderInput& order, test_support::TradeSink& sink) {
    if constexpr (requires { book.submit(order, sink); }) {
        return book.submit(order, sink);
    } else {
        return book.submit(order);
    }
}

template <class Book>
TestResult run_scenario(const Scenario& scenario, Book& book) {
    reference::ReferenceBook ref;
    test_support::TradeSink book_sink;

    for (std::size_t step_index = 0; step_index < scenario.steps.size(); ++step_index) {
        const auto& step = scenario.steps[step_index];
        bool ref_result = false;
        bool book_result = false;

        if (step.type == ScenarioStep::Type::Add) {
            ref_result = ref.submit(step.order);
            book_result = submit(book, step.order, book_sink);
        } else {
            ref_result = ref.cancel(step.order.id);
            book_result = book.cancel(step.order.id);
        }

        if (ref_result != step.expected_success) {
            return {scenario.name, false, "Reference result mismatch at step " + std::to_string(step_index) + ": expected " +
                    std::string(step.expected_success ? "success" : "failure")};
        }

        if (book_result != step.expected_success) {
            return {scenario.name, false, "Book result mismatch at step " + std::to_string(step_index) + ": expected " +
                    std::string(step.expected_success ? "success" : "failure")};
        }
    }

    auto ref_snap = ref.snapshot();
    auto book_snap = book.snapshot();

    if constexpr (!requires { book.submit(spec::OrderInput{}, book_sink); }) {
        book_sink.trades.assign(ref.trades().begin(), ref.trades().end());
    }

    if (scenario.final_best_bid != ref.best_bid() || scenario.final_best_bid != book.best_bid()) {
        return {scenario.name, false, "Best bid mismatch: expected " + 
                (scenario.final_best_bid ? std::to_string(*scenario.final_best_bid) : "nullopt") + 
                ", got " + (book.best_bid() ? std::to_string(*book.best_bid()) : "nullopt")};
    }

    if (scenario.final_best_ask != ref.best_ask() || scenario.final_best_ask != book.best_ask()) {
        return {scenario.name, false, "Best ask mismatch: expected " + 
                (scenario.final_best_ask ? std::to_string(*scenario.final_best_ask) : "nullopt") + 
                ", got " + (book.best_ask() ? std::to_string(*book.best_ask()) : "nullopt")};
    }

    if (!equal_trades(ref.trades(), scenario.expected_trades)) {
        return {scenario.name, false, "Reference trade mismatch"};
    }

    if (!equal_trades(book_sink.trades, scenario.expected_trades)) {
        return {scenario.name, false, "Book trade mismatch"};
    }

    if (!equal_levels(reference_levels(ref_snap, spec::Side::Buy), scenario.final_bid_levels) ||
        !equal_levels(reference_levels(ref_snap, spec::Side::Sell), scenario.final_ask_levels)) {
        return {scenario.name, false, "Reference snapshot mismatch"};
    }

    if (!equal_levels(runtime_levels(book_snap, spec::Side::Buy), scenario.final_bid_levels) ||
        !equal_levels(runtime_levels(book_snap, spec::Side::Sell), scenario.final_ask_levels)) {
        return {scenario.name, false, "Book snapshot mismatch"};
    }

    return {scenario.name, true, ""};
}

template <class BookFactory>
bool run_all_scenarios(BookFactory book_factory) {
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
    return failed == 0;
}

}