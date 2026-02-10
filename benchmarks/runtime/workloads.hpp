#pragma once

#include "../../include/lob/spec.hpp"
#include <vector>
#include <cstdint>
#include <random>

namespace lob::benchmarks {

struct Workload {
    std::vector<spec::OrderInput> submits;
    std::vector<spec::OrderId> cancels;
    std::vector<spec::OrderInput> mixed;
};

inline Workload generate_workload(std::size_t num_orders, double cancel_ratio, uint64_t seed,
                                  spec::Price min_price = 1, spec::Price max_price = 10000) {
    Workload w;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<spec::Price> price_dist(min_price, max_price);
    std::uniform_int_distribution<spec::Quantity> qty_dist(1, 100);
    std::uniform_int_distribution<int> side_dist(0, 1);
    std::uniform_real_distribution<double> cancel_dist(0.0, 1.0);

    spec::OrderId next_id = 1;
    std::vector<spec::OrderId> active_ids;

    for (std::size_t i = 0; i < num_orders; ++i) {
        spec::OrderInput order;
        order.id = next_id++;
        order.side = side_dist(rng) == 0 ? spec::Side::Buy : spec::Side::Sell;
        order.price = price_dist(rng);
        order.quantity = qty_dist(rng);

        w.submits.push_back(order);
        active_ids.push_back(order.id);

        if (!active_ids.empty() && cancel_dist(rng) < cancel_ratio) {
            std::uniform_int_distribution<std::size_t> idx_dist(0, active_ids.size() - 1);
            std::size_t cancel_idx = idx_dist(rng);
            w.cancels.push_back(active_ids[cancel_idx]);
            active_ids.erase(active_ids.begin() + cancel_idx);
        }
    }

    // Generate mixed workload (interleaved submits and cancels)
    w.mixed = w.submits;
    for (const auto& cancel_id : w.cancels) {
        spec::OrderInput cancel_order;
        cancel_order.id = cancel_id;
        cancel_order.side = spec::Side::Buy;
        cancel_order.price = 0;
        cancel_order.quantity = 0;
        w.mixed.push_back(cancel_order);
    }

    return w;
}

inline Workload generate_no_cross_workload(std::size_t num_orders, uint64_t seed,
                                           spec::Price min_price = 1, spec::Price max_price = 10000) {
    Workload w;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<spec::Price> price_dist(min_price, max_price);
    std::uniform_int_distribution<spec::Quantity> qty_dist(1, 100);

    spec::Price mid = (min_price + max_price) / 2;

    for (std::size_t i = 0; i < num_orders; ++i) {
        spec::OrderInput order;
        order.id = static_cast<spec::OrderId>(i + 1);
        order.side = (i % 2 == 0) ? spec::Side::Buy : spec::Side::Sell;
        order.price = order.side == spec::Side::Buy 
            ? std::uniform_int_distribution<spec::Price>(min_price, mid - 1)(rng)
            : std::uniform_int_distribution<spec::Price>(mid + 1, max_price)(rng);
        order.quantity = qty_dist(rng);
        w.submits.push_back(order);
    }
    return w;
}

inline Workload generate_crossing_workload(std::size_t num_orders, uint64_t seed,
                                           spec::Price min_price = 1, spec::Price max_price = 10000) {
    Workload w;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<spec::Price> price_dist(min_price, max_price);
    std::uniform_int_distribution<spec::Quantity> qty_dist(1, 100);

    spec::Price mid = (min_price + max_price) / 2;

    for (std::size_t i = 0; i < num_orders; ++i) {
        spec::OrderInput order;
        order.id = static_cast<spec::OrderId>(i + 1);
        order.side = (i % 2 == 0) ? spec::Side::Buy : spec::Side::Sell;
        order.price = order.side == spec::Side::Buy
            ? std::uniform_int_distribution<spec::Price>(mid, max_price)(rng)
            : std::uniform_int_distribution<spec::Price>(min_price, mid)(rng);
        order.quantity = qty_dist(rng);
        w.submits.push_back(order);
    }
    return w;
}

inline Workload generate_same_price_workload(std::size_t num_orders, spec::Price price,
                                             spec::Side side, uint64_t seed) {
    Workload w;
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<spec::Quantity> qty_dist(1, 100);

    for (std::size_t i = 0; i < num_orders; ++i) {
        spec::OrderInput order;
        order.id = static_cast<spec::OrderId>(i + 1);
        order.side = side;
        order.price = price;
        order.quantity = qty_dist(rng);
        w.submits.push_back(order);
    }
    return w;
}

inline Workload generate_evolution_workload(int stage, std::size_t num_orders, uint64_t seed,
                                            spec::Price min_price = 1, spec::Price max_price = 10000) {
    // Same workload for all evolution stages to ensure fair comparison
    return generate_workload(num_orders, 0.1, seed, min_price, max_price);
}

}