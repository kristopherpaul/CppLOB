#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 50
// PRICE_LEVELS_PER_SIDE: 10

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>, Order<Side::Buy, 100, 10, 4>, Order<Side::Buy, 100, 10, 5>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>, Order<Side::Buy, 99, 10, 102>, Order<Side::Buy, 99, 10, 103>, Order<Side::Buy, 99, 10, 104>, Order<Side::Buy, 99, 10, 105>>, PriceLevel<98, Order<Side::Buy, 98, 10, 201>, Order<Side::Buy, 98, 10, 202>, Order<Side::Buy, 98, 10, 203>, Order<Side::Buy, 98, 10, 204>, Order<Side::Buy, 98, 10, 205>>, PriceLevel<97, Order<Side::Buy, 97, 10, 301>, Order<Side::Buy, 97, 10, 302>, Order<Side::Buy, 97, 10, 303>, Order<Side::Buy, 97, 10, 304>, Order<Side::Buy, 97, 10, 305>>, PriceLevel<96, Order<Side::Buy, 96, 10, 401>, Order<Side::Buy, 96, 10, 402>, Order<Side::Buy, 96, 10, 403>, Order<Side::Buy, 96, 10, 404>, Order<Side::Buy, 96, 10, 405>>, PriceLevel<95, Order<Side::Buy, 95, 10, 501>, Order<Side::Buy, 95, 10, 502>, Order<Side::Buy, 95, 10, 503>, Order<Side::Buy, 95, 10, 504>, Order<Side::Buy, 95, 10, 505>>, PriceLevel<94, Order<Side::Buy, 94, 10, 601>, Order<Side::Buy, 94, 10, 602>, Order<Side::Buy, 94, 10, 603>, Order<Side::Buy, 94, 10, 604>, Order<Side::Buy, 94, 10, 605>>, PriceLevel<93, Order<Side::Buy, 93, 10, 701>, Order<Side::Buy, 93, 10, 702>, Order<Side::Buy, 93, 10, 703>, Order<Side::Buy, 93, 10, 704>, Order<Side::Buy, 93, 10, 705>>, PriceLevel<92, Order<Side::Buy, 92, 10, 801>, Order<Side::Buy, 92, 10, 802>, Order<Side::Buy, 92, 10, 803>, Order<Side::Buy, 92, 10, 804>, Order<Side::Buy, 92, 10, 805>>, PriceLevel<91, Order<Side::Buy, 91, 10, 901>, Order<Side::Buy, 91, 10, 902>, Order<Side::Buy, 91, 10, 903>, Order<Side::Buy, 91, 10, 904>, Order<Side::Buy, 91, 10, 905>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>, Order<Side::Sell, 101, 10, 1004>, Order<Side::Sell, 101, 10, 1005>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>, Order<Side::Sell, 102, 10, 1102>, Order<Side::Sell, 102, 10, 1103>, Order<Side::Sell, 102, 10, 1104>, Order<Side::Sell, 102, 10, 1105>>, PriceLevel<103, Order<Side::Sell, 103, 10, 1201>, Order<Side::Sell, 103, 10, 1202>, Order<Side::Sell, 103, 10, 1203>, Order<Side::Sell, 103, 10, 1204>, Order<Side::Sell, 103, 10, 1205>>, PriceLevel<104, Order<Side::Sell, 104, 10, 1301>, Order<Side::Sell, 104, 10, 1302>, Order<Side::Sell, 104, 10, 1303>, Order<Side::Sell, 104, 10, 1304>, Order<Side::Sell, 104, 10, 1305>>, PriceLevel<105, Order<Side::Sell, 105, 10, 1401>, Order<Side::Sell, 105, 10, 1402>, Order<Side::Sell, 105, 10, 1403>, Order<Side::Sell, 105, 10, 1404>, Order<Side::Sell, 105, 10, 1405>>, PriceLevel<106, Order<Side::Sell, 106, 10, 1501>, Order<Side::Sell, 106, 10, 1502>, Order<Side::Sell, 106, 10, 1503>, Order<Side::Sell, 106, 10, 1504>, Order<Side::Sell, 106, 10, 1505>>, PriceLevel<107, Order<Side::Sell, 107, 10, 1601>, Order<Side::Sell, 107, 10, 1602>, Order<Side::Sell, 107, 10, 1603>, Order<Side::Sell, 107, 10, 1604>, Order<Side::Sell, 107, 10, 1605>>, PriceLevel<108, Order<Side::Sell, 108, 10, 1701>, Order<Side::Sell, 108, 10, 1702>, Order<Side::Sell, 108, 10, 1703>, Order<Side::Sell, 108, 10, 1704>, Order<Side::Sell, 108, 10, 1705>>, PriceLevel<109, Order<Side::Sell, 109, 10, 1801>, Order<Side::Sell, 109, 10, 1802>, Order<Side::Sell, 109, 10, 1803>, Order<Side::Sell, 109, 10, 1804>, Order<Side::Sell, 109, 10, 1805>>, PriceLevel<110, Order<Side::Sell, 110, 10, 1901>, Order<Side::Sell, 110, 10, 1902>, Order<Side::Sell, 110, 10, 1903>, Order<Side::Sell, 110, 10, 1904>, Order<Side::Sell, 110, 10, 1905>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}