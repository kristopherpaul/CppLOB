#pragma once

#include "../../include/lob/spec.hpp"
#include "../../tests/reference/reference_book.hpp"
#include <vector>
#include <optional>
#include <string>

namespace lob::conformance {

struct ScenarioStep {
    enum class Type { Add, Cancel } type;
    spec::OrderInput order;
    bool expected_success;
};

struct ExpectedTrade {
    spec::OrderId aggressor;
    spec::OrderId resting;
    spec::Price price;
    spec::Quantity quantity;
};

struct Scenario {
    std::string name;
    std::vector<ScenarioStep> steps;
    std::vector<ExpectedTrade> expected_trades;
    std::optional<spec::Price> final_best_bid;
    std::optional<spec::Price> final_best_ask;
    std::vector<std::pair<spec::Price, std::vector<spec::OrderInput>>> final_bid_levels;
    std::vector<std::pair<spec::Price, std::vector<spec::OrderInput>>> final_ask_levels;
};

inline std::vector<Scenario> all_scenarios() {
    std::vector<Scenario> scenarios;

    scenarios.push_back(Scenario{
        "empty_book",
        {},
        {},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "non_crossing_bid_ask",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Buy, 100, 10}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 105, 10}, true},
        },
        {},
        100,
        105,
        {{100, {{1, spec::Side::Buy, 100, 10}}}},
        {{105, {{2, spec::Side::Sell, 105, 10}}}}
    });

    scenarios.push_back(Scenario{
        "full_fill_buy_crosses_ask",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Buy, 100, 10}, true},
        },
        {{2, 1, 100, 10}},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "partial_fill_buy_crosses_ask",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Buy, 100, 5}, true},
        },
        {{2, 1, 100, 5}},
        std::nullopt,
        100,
        {},
        {{100, {{1, spec::Side::Sell, 100, 5}}}}
    });

    scenarios.push_back(Scenario{
        "multiple_price_levels",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 101, 10}, true},
            {ScenarioStep::Type::Add, {3, spec::Side::Buy, 101, 15}, true},
        },
        {{3, 1, 100, 10}, {3, 2, 101, 5}},
        std::nullopt,
        101,
        {},
        {{101, {{2, spec::Side::Sell, 101, 5}}}}
    });

    scenarios.push_back(Scenario{
        "fifo_at_same_price",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {3, spec::Side::Buy, 100, 7}, true},
        },
        {{3, 1, 100, 5}, {3, 2, 100, 2}},
        std::nullopt,
        100,
        {},
        {{100, {{2, spec::Side::Sell, 100, 3}}}}
    });

    scenarios.push_back(Scenario{
        "cancel_head",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Cancel, {1, spec::Side::Sell, 0, 0}, true},
        },
        {},
        std::nullopt,
        100,
        {},
        {{100, {{2, spec::Side::Sell, 100, 10}}}}
    });

    scenarios.push_back(Scenario{
        "cancel_middle",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {3, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Cancel, {2, spec::Side::Sell, 0, 0}, true},
        },
        {},
        std::nullopt,
        100,
        {},
        {{100, {{1, spec::Side::Sell, 100, 5}, {3, spec::Side::Sell, 100, 5}}}}
    });

    scenarios.push_back(Scenario{
        "cancel_tail",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Cancel, {2, spec::Side::Sell, 0, 0}, true},
        },
        {},
        std::nullopt,
        100,
        {},
        {{100, {{1, spec::Side::Sell, 100, 5}}}}
    });

    scenarios.push_back(Scenario{
        "cancel_unknown",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Cancel, {999, spec::Side::Sell, 0, 0}, false},
        },
        {},
        std::nullopt,
        100,
        {},
        {{100, {{1, spec::Side::Sell, 100, 10}}}}
    });

    scenarios.push_back(Scenario{
        "repeated_cancel",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, true},
            {ScenarioStep::Type::Cancel, {1, spec::Side::Sell, 0, 0}, true},
            {ScenarioStep::Type::Cancel, {1, spec::Side::Sell, 0, 0}, false},
        },
        {},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "level_deletion_after_final_fill",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Buy, 100, 5}, true},
        },
        {{2, 1, 100, 5}},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "level_deletion_after_final_cancel",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Cancel, {1, spec::Side::Sell, 0, 0}, true},
        },
        {},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "multiple_consecutive_matches",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 3}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Sell, 101, 3}, true},
            {ScenarioStep::Type::Add, {3, spec::Side::Sell, 102, 3}, true},
            {ScenarioStep::Type::Add, {4, spec::Side::Buy, 102, 7}, true},
        },
        {{4, 1, 100, 3}, {4, 2, 101, 3}, {4, 3, 102, 1}},
        std::nullopt,
        102,
        {},
        {{102, {{3, spec::Side::Sell, 102, 2}}}}
    });

    scenarios.push_back(Scenario{
        "book_becomes_empty",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Buy, 100, 5}, true},
        },
        {{2, 1, 100, 5}},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "book_becomes_one_sided_bid",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Buy, 100, 3}, true},
        },
        {{2, 1, 100, 3}},
        std::nullopt,
        100,
        {},
        {{100, {{1, spec::Side::Sell, 100, 2}}}}
    });

    scenarios.push_back(Scenario{
        "book_becomes_one_sided_ask",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 5}, true},
            {ScenarioStep::Type::Add, {2, spec::Side::Buy, 100, 7}, true},
        },
        {{2, 1, 100, 5}},
        100,
        std::nullopt,
        {{100, {{2, spec::Side::Buy, 100, 2}}}},
        {}
    });

    scenarios.push_back(Scenario{
        "invalid_order_zero_quantity",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Buy, 100, 0}, false},
        },
        {},
        std::nullopt,
        std::nullopt,
        {},
        {}
    });

    scenarios.push_back(Scenario{
        "invalid_order_duplicate_id",
        {
            {ScenarioStep::Type::Add, {1, spec::Side::Buy, 100, 10}, true},
            {ScenarioStep::Type::Add, {1, spec::Side::Sell, 100, 10}, false},
        },
        {},
        100,
        std::nullopt,
        {{100, {{1, spec::Side::Buy, 100, 10}}}},
        {}
    });

    return scenarios;
}

}