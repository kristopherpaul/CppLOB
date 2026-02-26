#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 20
// PRICE_LEVELS_PER_SIDE: 10

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>, Order<Side::Buy, 99, 10, 102>>, PriceLevel<98, Order<Side::Buy, 98, 10, 201>, Order<Side::Buy, 98, 10, 202>>, PriceLevel<97, Order<Side::Buy, 97, 10, 301>, Order<Side::Buy, 97, 10, 302>>, PriceLevel<96, Order<Side::Buy, 96, 10, 401>, Order<Side::Buy, 96, 10, 402>>, PriceLevel<95, Order<Side::Buy, 95, 10, 501>, Order<Side::Buy, 95, 10, 502>>, PriceLevel<94, Order<Side::Buy, 94, 10, 601>, Order<Side::Buy, 94, 10, 602>>, PriceLevel<93, Order<Side::Buy, 93, 10, 701>, Order<Side::Buy, 93, 10, 702>>, PriceLevel<92, Order<Side::Buy, 92, 10, 801>, Order<Side::Buy, 92, 10, 802>>, PriceLevel<91, Order<Side::Buy, 91, 10, 901>, Order<Side::Buy, 91, 10, 902>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>, Order<Side::Sell, 102, 10, 1102>>, PriceLevel<103, Order<Side::Sell, 103, 10, 1201>, Order<Side::Sell, 103, 10, 1202>>, PriceLevel<104, Order<Side::Sell, 104, 10, 1301>, Order<Side::Sell, 104, 10, 1302>>, PriceLevel<105, Order<Side::Sell, 105, 10, 1401>, Order<Side::Sell, 105, 10, 1402>>, PriceLevel<106, Order<Side::Sell, 106, 10, 1501>, Order<Side::Sell, 106, 10, 1502>>, PriceLevel<107, Order<Side::Sell, 107, 10, 1601>, Order<Side::Sell, 107, 10, 1602>>, PriceLevel<108, Order<Side::Sell, 108, 10, 1701>, Order<Side::Sell, 108, 10, 1702>>, PriceLevel<109, Order<Side::Sell, 109, 10, 1801>, Order<Side::Sell, 109, 10, 1802>>, PriceLevel<110, Order<Side::Sell, 110, 10, 1901>, Order<Side::Sell, 110, 10, 1902>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}