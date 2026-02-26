#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 50
// PRICE_LEVELS_PER_SIDE: 2

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>, Order<Side::Buy, 100, 10, 4>, Order<Side::Buy, 100, 10, 5>, Order<Side::Buy, 100, 10, 6>, Order<Side::Buy, 100, 10, 7>, Order<Side::Buy, 100, 10, 8>, Order<Side::Buy, 100, 10, 9>, Order<Side::Buy, 100, 10, 10>, Order<Side::Buy, 100, 10, 11>, Order<Side::Buy, 100, 10, 12>, Order<Side::Buy, 100, 10, 13>, Order<Side::Buy, 100, 10, 14>, Order<Side::Buy, 100, 10, 15>, Order<Side::Buy, 100, 10, 16>, Order<Side::Buy, 100, 10, 17>, Order<Side::Buy, 100, 10, 18>, Order<Side::Buy, 100, 10, 19>, Order<Side::Buy, 100, 10, 20>, Order<Side::Buy, 100, 10, 21>, Order<Side::Buy, 100, 10, 22>, Order<Side::Buy, 100, 10, 23>, Order<Side::Buy, 100, 10, 24>, Order<Side::Buy, 100, 10, 25>>, PriceLevel<99, Order<Side::Buy, 99, 10, 101>, Order<Side::Buy, 99, 10, 102>, Order<Side::Buy, 99, 10, 103>, Order<Side::Buy, 99, 10, 104>, Order<Side::Buy, 99, 10, 105>, Order<Side::Buy, 99, 10, 106>, Order<Side::Buy, 99, 10, 107>, Order<Side::Buy, 99, 10, 108>, Order<Side::Buy, 99, 10, 109>, Order<Side::Buy, 99, 10, 110>, Order<Side::Buy, 99, 10, 111>, Order<Side::Buy, 99, 10, 112>, Order<Side::Buy, 99, 10, 113>, Order<Side::Buy, 99, 10, 114>, Order<Side::Buy, 99, 10, 115>, Order<Side::Buy, 99, 10, 116>, Order<Side::Buy, 99, 10, 117>, Order<Side::Buy, 99, 10, 118>, Order<Side::Buy, 99, 10, 119>, Order<Side::Buy, 99, 10, 120>, Order<Side::Buy, 99, 10, 121>, Order<Side::Buy, 99, 10, 122>, Order<Side::Buy, 99, 10, 123>, Order<Side::Buy, 99, 10, 124>, Order<Side::Buy, 99, 10, 125>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>, Order<Side::Sell, 101, 10, 1004>, Order<Side::Sell, 101, 10, 1005>, Order<Side::Sell, 101, 10, 1006>, Order<Side::Sell, 101, 10, 1007>, Order<Side::Sell, 101, 10, 1008>, Order<Side::Sell, 101, 10, 1009>, Order<Side::Sell, 101, 10, 1010>, Order<Side::Sell, 101, 10, 1011>, Order<Side::Sell, 101, 10, 1012>, Order<Side::Sell, 101, 10, 1013>, Order<Side::Sell, 101, 10, 1014>, Order<Side::Sell, 101, 10, 1015>, Order<Side::Sell, 101, 10, 1016>, Order<Side::Sell, 101, 10, 1017>, Order<Side::Sell, 101, 10, 1018>, Order<Side::Sell, 101, 10, 1019>, Order<Side::Sell, 101, 10, 1020>, Order<Side::Sell, 101, 10, 1021>, Order<Side::Sell, 101, 10, 1022>, Order<Side::Sell, 101, 10, 1023>, Order<Side::Sell, 101, 10, 1024>, Order<Side::Sell, 101, 10, 1025>>, PriceLevel<102, Order<Side::Sell, 102, 10, 1101>, Order<Side::Sell, 102, 10, 1102>, Order<Side::Sell, 102, 10, 1103>, Order<Side::Sell, 102, 10, 1104>, Order<Side::Sell, 102, 10, 1105>, Order<Side::Sell, 102, 10, 1106>, Order<Side::Sell, 102, 10, 1107>, Order<Side::Sell, 102, 10, 1108>, Order<Side::Sell, 102, 10, 1109>, Order<Side::Sell, 102, 10, 1110>, Order<Side::Sell, 102, 10, 1111>, Order<Side::Sell, 102, 10, 1112>, Order<Side::Sell, 102, 10, 1113>, Order<Side::Sell, 102, 10, 1114>, Order<Side::Sell, 102, 10, 1115>, Order<Side::Sell, 102, 10, 1116>, Order<Side::Sell, 102, 10, 1117>, Order<Side::Sell, 102, 10, 1118>, Order<Side::Sell, 102, 10, 1119>, Order<Side::Sell, 102, 10, 1120>, Order<Side::Sell, 102, 10, 1121>, Order<Side::Sell, 102, 10, 1122>, Order<Side::Sell, 102, 10, 1123>, Order<Side::Sell, 102, 10, 1124>, Order<Side::Sell, 102, 10, 1125>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}