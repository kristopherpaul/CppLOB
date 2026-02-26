#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 20
// PRICE_LEVELS_PER_SIDE: 1

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>, Order<Side::Buy, 100, 10, 4>, Order<Side::Buy, 100, 10, 5>, Order<Side::Buy, 100, 10, 6>, Order<Side::Buy, 100, 10, 7>, Order<Side::Buy, 100, 10, 8>, Order<Side::Buy, 100, 10, 9>, Order<Side::Buy, 100, 10, 10>, Order<Side::Buy, 100, 10, 11>, Order<Side::Buy, 100, 10, 12>, Order<Side::Buy, 100, 10, 13>, Order<Side::Buy, 100, 10, 14>, Order<Side::Buy, 100, 10, 15>, Order<Side::Buy, 100, 10, 16>, Order<Side::Buy, 100, 10, 17>, Order<Side::Buy, 100, 10, 18>, Order<Side::Buy, 100, 10, 19>, Order<Side::Buy, 100, 10, 20>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>, Order<Side::Sell, 101, 10, 1004>, Order<Side::Sell, 101, 10, 1005>, Order<Side::Sell, 101, 10, 1006>, Order<Side::Sell, 101, 10, 1007>, Order<Side::Sell, 101, 10, 1008>, Order<Side::Sell, 101, 10, 1009>, Order<Side::Sell, 101, 10, 1010>, Order<Side::Sell, 101, 10, 1011>, Order<Side::Sell, 101, 10, 1012>, Order<Side::Sell, 101, 10, 1013>, Order<Side::Sell, 101, 10, 1014>, Order<Side::Sell, 101, 10, 1015>, Order<Side::Sell, 101, 10, 1016>, Order<Side::Sell, 101, 10, 1017>, Order<Side::Sell, 101, 10, 1018>, Order<Side::Sell, 101, 10, 1019>, Order<Side::Sell, 101, 10, 1020>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}