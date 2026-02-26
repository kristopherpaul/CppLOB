#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 50
// PRICE_LEVELS_PER_SIDE: 20

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>, Order<Side::Buy, 99, 10, 102>, Order<Side::Buy, 99, 10, 103>>, PriceLevel<98, Order<Side::Buy, 98, 10, 201>, Order<Side::Buy, 98, 10, 202>, Order<Side::Buy, 98, 10, 203>>, PriceLevel<97, Order<Side::Buy, 97, 10, 301>, Order<Side::Buy, 97, 10, 302>, Order<Side::Buy, 97, 10, 303>>, PriceLevel<96, Order<Side::Buy, 96, 10, 401>, Order<Side::Buy, 96, 10, 402>, Order<Side::Buy, 96, 10, 403>>, PriceLevel<95, Order<Side::Buy, 95, 10, 501>, Order<Side::Buy, 95, 10, 502>, Order<Side::Buy, 95, 10, 503>>, PriceLevel<94, Order<Side::Buy, 94, 10, 601>, Order<Side::Buy, 94, 10, 602>, Order<Side::Buy, 94, 10, 603>>, PriceLevel<93, Order<Side::Buy, 93, 10, 701>, Order<Side::Buy, 93, 10, 702>, Order<Side::Buy, 93, 10, 703>>, PriceLevel<92, Order<Side::Buy, 92, 10, 801>, Order<Side::Buy, 92, 10, 802>, Order<Side::Buy, 92, 10, 803>>, PriceLevel<91, Order<Side::Buy, 91, 10, 901>, Order<Side::Buy, 91, 10, 902>, Order<Side::Buy, 91, 10, 903>>, PriceLevel<90, Order<Side::Buy, 90, 10, 1001>, Order<Side::Buy, 90, 10, 1002>>, PriceLevel<89, Order<Side::Buy, 89, 10, 1101>, Order<Side::Buy, 89, 10, 1102>>, PriceLevel<88, Order<Side::Buy, 88, 10, 1201>, Order<Side::Buy, 88, 10, 1202>>, PriceLevel<87, Order<Side::Buy, 87, 10, 1301>, Order<Side::Buy, 87, 10, 1302>>, PriceLevel<86, Order<Side::Buy, 86, 10, 1401>, Order<Side::Buy, 86, 10, 1402>>, PriceLevel<85, Order<Side::Buy, 85, 10, 1501>, Order<Side::Buy, 85, 10, 1502>>, PriceLevel<84, Order<Side::Buy, 84, 10, 1601>, Order<Side::Buy, 84, 10, 1602>>, PriceLevel<83, Order<Side::Buy, 83, 10, 1701>, Order<Side::Buy, 83, 10, 1702>>, PriceLevel<82, Order<Side::Buy, 82, 10, 1801>, Order<Side::Buy, 82, 10, 1802>>, PriceLevel<81, Order<Side::Buy, 81, 10, 1901>, Order<Side::Buy, 81, 10, 1902>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>, Order<Side::Sell, 102, 10, 1102>, Order<Side::Sell, 102, 10, 1103>>, PriceLevel<103, Order<Side::Sell, 103, 10, 1201>, Order<Side::Sell, 103, 10, 1202>, Order<Side::Sell, 103, 10, 1203>>, PriceLevel<104, Order<Side::Sell, 104, 10, 1301>, Order<Side::Sell, 104, 10, 1302>, Order<Side::Sell, 104, 10, 1303>>, PriceLevel<105, Order<Side::Sell, 105, 10, 1401>, Order<Side::Sell, 105, 10, 1402>, Order<Side::Sell, 105, 10, 1403>>, PriceLevel<106, Order<Side::Sell, 106, 10, 1501>, Order<Side::Sell, 106, 10, 1502>, Order<Side::Sell, 106, 10, 1503>>, PriceLevel<107, Order<Side::Sell, 107, 10, 1601>, Order<Side::Sell, 107, 10, 1602>, Order<Side::Sell, 107, 10, 1603>>, PriceLevel<108, Order<Side::Sell, 108, 10, 1701>, Order<Side::Sell, 108, 10, 1702>, Order<Side::Sell, 108, 10, 1703>>, PriceLevel<109, Order<Side::Sell, 109, 10, 1801>, Order<Side::Sell, 109, 10, 1802>, Order<Side::Sell, 109, 10, 1803>>, PriceLevel<110, Order<Side::Sell, 110, 10, 1901>, Order<Side::Sell, 110, 10, 1902>, Order<Side::Sell, 110, 10, 1903>>, PriceLevel<111, Order<Side::Sell, 111, 10, 2001>, Order<Side::Sell, 111, 10, 2002>>, PriceLevel<112, Order<Side::Sell, 112, 10, 2101>, Order<Side::Sell, 112, 10, 2102>>, PriceLevel<113, Order<Side::Sell, 113, 10, 2201>, Order<Side::Sell, 113, 10, 2202>>, PriceLevel<114, Order<Side::Sell, 114, 10, 2301>, Order<Side::Sell, 114, 10, 2302>>, PriceLevel<115, Order<Side::Sell, 115, 10, 2401>, Order<Side::Sell, 115, 10, 2402>>, PriceLevel<116, Order<Side::Sell, 116, 10, 2501>, Order<Side::Sell, 116, 10, 2502>>, PriceLevel<117, Order<Side::Sell, 117, 10, 2601>, Order<Side::Sell, 117, 10, 2602>>, PriceLevel<118, Order<Side::Sell, 118, 10, 2701>, Order<Side::Sell, 118, 10, 2702>>, PriceLevel<119, Order<Side::Sell, 119, 10, 2801>, Order<Side::Sell, 119, 10, 2802>>, PriceLevel<120, Order<Side::Sell, 120, 10, 2901>, Order<Side::Sell, 120, 10, 2902>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}