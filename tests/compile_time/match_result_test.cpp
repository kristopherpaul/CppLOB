#include <compile_time/match.hpp>
#include <compile_time/result.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

using Ask1 = Order<Side::Sell, 101, 5, 10>;
using Ask2 = Order<Side::Sell, 101, 7, 11>;
using Ask3 = Order<Side::Sell, 105, 4, 12>;
using Asks = AddOrder_t<AddOrder_t<AddOrder_t<EmptyBook, Ask1>, Ask2>, Ask3>;
using Buy = Order<Side::Buy, 102, 8, 20>;
using Result = Submit_t<Asks, Buy>;
using FirstTrade = At_t<0, typename Result::trades>;
using SecondTrade = At_t<1, typename Result::trades>;

static_assert(Result::trade_count == 2);
static_assert(FirstTrade::aggressor == 20);
static_assert(FirstTrade::resting == 10);
static_assert(FirstTrade::price == 101);
static_assert(FirstTrade::quantity == 5);
static_assert(SecondTrade::resting == 11);
static_assert(SecondTrade::quantity == 3);
static_assert(Result::book::asks::best_level::best_order::id == 11);
static_assert(Result::book::asks::best_level::best_order::quantity == 4);

using Partial = Submit_t<
    AddOrder_t<EmptyBook, Order<Side::Sell, 100, 10, 30>>,
    Order<Side::Buy, 100, 4, 31>>;
static_assert(Partial::book::asks::best_level::best_order::quantity == 6);

using SellResult = Submit_t<
    AddOrder_t<EmptyBook, Order<Side::Buy, 99, 6, 40>>,
    Order<Side::Sell, 98, 2, 41>>;
static_assert(SellResult::trade_count == 1);
static_assert(At_t<0, typename SellResult::trades>::price == 99);
static_assert(SellResult::book::bids::best_level::best_order::quantity == 4);

using Remainder = Submit_t<EmptyBook, Order<Side::Buy, 100, 5, 50>>;
static_assert(Remainder::trade_count == 0);
static_assert(Remainder::remainder::id == 50);
static_assert(Remainder::book::bids::best_level::best_order::quantity == 5);

struct Sink {
    OrderId aggressor = 0;
    OrderId resting = 0;
    Quantity quantity = 0;

    constexpr void on_trade(const lob::spec::Trade& trade) noexcept {
        aggressor = trade.aggressor;
        resting = trade.resting;
        quantity = trade.quantity;
    }
};

constexpr Sink collect_trade() {
    Sink sink{};
    Result::apply_trades(sink);
    return sink;
}

constexpr auto trade = collect_trade();
static_assert(trade.aggressor == 20);
static_assert(trade.resting == 11);
static_assert(trade.quantity == 3);

constexpr auto view = Result::runtime_view();
static_assert(view.asks.level_count == 2);
static_assert(view.asks.levels[0].price == 101);

int main() {
    return 0;
}
