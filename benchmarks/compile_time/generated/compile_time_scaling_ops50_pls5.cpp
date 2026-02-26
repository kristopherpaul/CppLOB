#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 50
// PRICE_LEVELS_PER_SIDE: 5

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>, Order<Side::Buy, 100, 10, 4>, Order<Side::Buy, 100, 10, 5>, Order<Side::Buy, 100, 10, 6>, Order<Side::Buy, 100, 10, 7>, Order<Side::Buy, 100, 10, 8>, Order<Side::Buy, 100, 10, 9>, Order<Side::Buy, 100, 10, 10>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>, Order<Side::Buy, 99, 10, 102>, Order<Side::Buy, 99, 10, 103>, Order<Side::Buy, 99, 10, 104>, Order<Side::Buy, 99, 10, 105>, Order<Side::Buy, 99, 10, 106>, Order<Side::Buy, 99, 10, 107>, Order<Side::Buy, 99, 10, 108>, Order<Side::Buy, 99, 10, 109>, Order<Side::Buy, 99, 10, 110>>, PriceLevel<98, Order<Side::Buy, 98, 10, 201>, Order<Side::Buy, 98, 10, 202>, Order<Side::Buy, 98, 10, 203>, Order<Side::Buy, 98, 10, 204>, Order<Side::Buy, 98, 10, 205>, Order<Side::Buy, 98, 10, 206>, Order<Side::Buy, 98, 10, 207>, Order<Side::Buy, 98, 10, 208>, Order<Side::Buy, 98, 10, 209>, Order<Side::Buy, 98, 10, 210>>, PriceLevel<97, Order<Side::Buy, 97, 10, 301>, Order<Side::Buy, 97, 10, 302>, Order<Side::Buy, 97, 10, 303>, Order<Side::Buy, 97, 10, 304>, Order<Side::Buy, 97, 10, 305>, Order<Side::Buy, 97, 10, 306>, Order<Side::Buy, 97, 10, 307>, Order<Side::Buy, 97, 10, 308>, Order<Side::Buy, 97, 10, 309>, Order<Side::Buy, 97, 10, 310>>, PriceLevel<96, Order<Side::Buy, 96, 10, 401>, Order<Side::Buy, 96, 10, 402>, Order<Side::Buy, 96, 10, 403>, Order<Side::Buy, 96, 10, 404>, Order<Side::Buy, 96, 10, 405>, Order<Side::Buy, 96, 10, 406>, Order<Side::Buy, 96, 10, 407>, Order<Side::Buy, 96, 10, 408>, Order<Side::Buy, 96, 10, 409>, Order<Side::Buy, 96, 10, 410>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>, Order<Side::Sell, 101, 10, 1004>, Order<Side::Sell, 101, 10, 1005>, Order<Side::Sell, 101, 10, 1006>, Order<Side::Sell, 101, 10, 1007>, Order<Side::Sell, 101, 10, 1008>, Order<Side::Sell, 101, 10, 1009>, Order<Side::Sell, 101, 10, 1010>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>, Order<Side::Sell, 102, 10, 1102>, Order<Side::Sell, 102, 10, 1103>, Order<Side::Sell, 102, 10, 1104>, Order<Side::Sell, 102, 10, 1105>, Order<Side::Sell, 102, 10, 1106>, Order<Side::Sell, 102, 10, 1107>, Order<Side::Sell, 102, 10, 1108>, Order<Side::Sell, 102, 10, 1109>, Order<Side::Sell, 102, 10, 1110>>, PriceLevel<103, Order<Side::Sell, 103, 10, 1201>, Order<Side::Sell, 103, 10, 1202>, Order<Side::Sell, 103, 10, 1203>, Order<Side::Sell, 103, 10, 1204>, Order<Side::Sell, 103, 10, 1205>, Order<Side::Sell, 103, 10, 1206>, Order<Side::Sell, 103, 10, 1207>, Order<Side::Sell, 103, 10, 1208>, Order<Side::Sell, 103, 10, 1209>, Order<Side::Sell, 103, 10, 1210>>, PriceLevel<104, Order<Side::Sell, 104, 10, 1301>, Order<Side::Sell, 104, 10, 1302>, Order<Side::Sell, 104, 10, 1303>, Order<Side::Sell, 104, 10, 1304>, Order<Side::Sell, 104, 10, 1305>, Order<Side::Sell, 104, 10, 1306>, Order<Side::Sell, 104, 10, 1307>, Order<Side::Sell, 104, 10, 1308>, Order<Side::Sell, 104, 10, 1309>, Order<Side::Sell, 104, 10, 1310>>, PriceLevel<105, Order<Side::Sell, 105, 10, 1401>, Order<Side::Sell, 105, 10, 1402>, Order<Side::Sell, 105, 10, 1403>, Order<Side::Sell, 105, 10, 1404>, Order<Side::Sell, 105, 10, 1405>, Order<Side::Sell, 105, 10, 1406>, Order<Side::Sell, 105, 10, 1407>, Order<Side::Sell, 105, 10, 1408>, Order<Side::Sell, 105, 10, 1409>, Order<Side::Sell, 105, 10, 1410>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}