#pragma once

#include "lob/spec.hpp"
#include <vector>

namespace lob::test_support {

struct TradeSink {
    std::vector<spec::Trade> trades;

    void on_trade(const spec::Trade& trade) noexcept {
        trades.push_back(trade);
    }
};

}
