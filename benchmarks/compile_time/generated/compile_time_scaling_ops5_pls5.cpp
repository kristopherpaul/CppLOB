#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 5
// PRICE_LEVELS_PER_SIDE: 5

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>>, PriceLevel<98, Order<Side::Buy, 98, 10, 201>>, PriceLevel<97, Order<Side::Buy, 97, 10, 301>>, PriceLevel<96, Order<Side::Buy, 96, 10, 401>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>>, PriceLevel<103, Order<Side::Sell, 103, 10, 1201>>, PriceLevel<104, Order<Side::Sell, 104, 10, 1301>>, PriceLevel<105, Order<Side::Sell, 105, 10, 1401>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}