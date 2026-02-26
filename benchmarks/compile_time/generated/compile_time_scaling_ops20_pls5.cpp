#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 20
// PRICE_LEVELS_PER_SIDE: 5

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>, Order<Side::Buy, 100, 10, 4>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>, Order<Side::Buy, 99, 10, 102>, Order<Side::Buy, 99, 10, 103>, Order<Side::Buy, 99, 10, 104>>, PriceLevel<98, Order<Side::Buy, 98, 10, 201>, Order<Side::Buy, 98, 10, 202>, Order<Side::Buy, 98, 10, 203>, Order<Side::Buy, 98, 10, 204>>, PriceLevel<97, Order<Side::Buy, 97, 10, 301>, Order<Side::Buy, 97, 10, 302>, Order<Side::Buy, 97, 10, 303>, Order<Side::Buy, 97, 10, 304>>, PriceLevel<96, Order<Side::Buy, 96, 10, 401>, Order<Side::Buy, 96, 10, 402>, Order<Side::Buy, 96, 10, 403>, Order<Side::Buy, 96, 10, 404>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>, Order<Side::Sell, 101, 10, 1004>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>, Order<Side::Sell, 102, 10, 1102>, Order<Side::Sell, 102, 10, 1103>, Order<Side::Sell, 102, 10, 1104>>, PriceLevel<103, Order<Side::Sell, 103, 10, 1201>, Order<Side::Sell, 103, 10, 1202>, Order<Side::Sell, 103, 10, 1203>, Order<Side::Sell, 103, 10, 1204>>, PriceLevel<104, Order<Side::Sell, 104, 10, 1301>, Order<Side::Sell, 104, 10, 1302>, Order<Side::Sell, 104, 10, 1303>, Order<Side::Sell, 104, 10, 1304>>, PriceLevel<105, Order<Side::Sell, 105, 10, 1401>, Order<Side::Sell, 105, 10, 1402>, Order<Side::Sell, 105, 10, 1403>, Order<Side::Sell, 105, 10, 1404>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}