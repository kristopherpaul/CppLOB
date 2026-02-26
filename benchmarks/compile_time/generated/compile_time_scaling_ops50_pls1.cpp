#include <compile_time/book.hpp>
#include <compile_time/match.hpp>
#include <compile_time/order.hpp>
#include <compile_time/type_list.hpp>
#include <lob/spec.hpp>

using namespace lob::compile_time;
using namespace lob::spec;

// Compile-time benchmark configuration
// ORDERS_PER_SIDE: 50
// PRICE_LEVELS_PER_SIDE: 1

using InitialBook = Book<BidBook<PriceLevel<100, Order<Side::Buy, 100, 10, 1>, Order<Side::Buy, 100, 10, 2>, Order<Side::Buy, 100, 10, 3>, Order<Side::Buy, 100, 10, 4>, Order<Side::Buy, 100, 10, 5>, Order<Side::Buy, 100, 10, 6>, Order<Side::Buy, 100, 10, 7>, Order<Side::Buy, 100, 10, 8>, Order<Side::Buy, 100, 10, 9>, Order<Side::Buy, 100, 10, 10>, Order<Side::Buy, 100, 10, 11>, Order<Side::Buy, 100, 10, 12>, Order<Side::Buy, 100, 10, 13>, Order<Side::Buy, 100, 10, 14>, Order<Side::Buy, 100, 10, 15>, Order<Side::Buy, 100, 10, 16>, Order<Side::Buy, 100, 10, 17>, Order<Side::Buy, 100, 10, 18>, Order<Side::Buy, 100, 10, 19>, Order<Side::Buy, 100, 10, 20>, Order<Side::Buy, 100, 10, 21>, Order<Side::Buy, 100, 10, 22>, Order<Side::Buy, 100, 10, 23>, Order<Side::Buy, 100, 10, 24>, Order<Side::Buy, 100, 10, 25>, Order<Side::Buy, 100, 10, 26>, Order<Side::Buy, 100, 10, 27>, Order<Side::Buy, 100, 10, 28>, Order<Side::Buy, 100, 10, 29>, Order<Side::Buy, 100, 10, 30>, Order<Side::Buy, 100, 10, 31>, Order<Side::Buy, 100, 10, 32>, Order<Side::Buy, 100, 10, 33>, Order<Side::Buy, 100, 10, 34>, Order<Side::Buy, 100, 10, 35>, Order<Side::Buy, 100, 10, 36>, Order<Side::Buy, 100, 10, 37>, Order<Side::Buy, 100, 10, 38>, Order<Side::Buy, 100, 10, 39>, Order<Side::Buy, 100, 10, 40>, Order<Side::Buy, 100, 10, 41>, Order<Side::Buy, 100, 10, 42>, Order<Side::Buy, 100, 10, 43>, Order<Side::Buy, 100, 10, 44>, Order<Side::Buy, 100, 10, 45>, Order<Side::Buy, 100, 10, 46>, Order<Side::Buy, 100, 10, 47>, Order<Side::Buy, 100, 10, 48>, Order<Side::Buy, 100, 10, 49>, Order<Side::Buy, 100, 10, 50>>>, AskBook<PriceLevel<101, Order<Side::Sell, 101, 10, 1001>, Order<Side::Sell, 101, 10, 1002>, Order<Side::Sell, 101, 10, 1003>, Order<Side::Sell, 101, 10, 1004>, Order<Side::Sell, 101, 10, 1005>, Order<Side::Sell, 101, 10, 1006>, Order<Side::Sell, 101, 10, 1007>, Order<Side::Sell, 101, 10, 1008>, Order<Side::Sell, 101, 10, 1009>, Order<Side::Sell, 101, 10, 1010>, Order<Side::Sell, 101, 10, 1011>, Order<Side::Sell, 101, 10, 1012>, Order<Side::Sell, 101, 10, 1013>, Order<Side::Sell, 101, 10, 1014>, Order<Side::Sell, 101, 10, 1015>, Order<Side::Sell, 101, 10, 1016>, Order<Side::Sell, 101, 10, 1017>, Order<Side::Sell, 101, 10, 1018>, Order<Side::Sell, 101, 10, 1019>, Order<Side::Sell, 101, 10, 1020>, Order<Side::Sell, 101, 10, 1021>, Order<Side::Sell, 101, 10, 1022>, Order<Side::Sell, 101, 10, 1023>, Order<Side::Sell, 101, 10, 1024>, Order<Side::Sell, 101, 10, 1025>, Order<Side::Sell, 101, 10, 1026>, Order<Side::Sell, 101, 10, 1027>, Order<Side::Sell, 101, 10, 1028>, Order<Side::Sell, 101, 10, 1029>, Order<Side::Sell, 101, 10, 1030>, Order<Side::Sell, 101, 10, 1031>, Order<Side::Sell, 101, 10, 1032>, Order<Side::Sell, 101, 10, 1033>, Order<Side::Sell, 101, 10, 1034>, Order<Side::Sell, 101, 10, 1035>, Order<Side::Sell, 101, 10, 1036>, Order<Side::Sell, 101, 10, 1037>, Order<Side::Sell, 101, 10, 1038>, Order<Side::Sell, 101, 10, 1039>, Order<Side::Sell, 101, 10, 1040>, Order<Side::Sell, 101, 10, 1041>, Order<Side::Sell, 101, 10, 1042>, Order<Side::Sell, 101, 10, 1043>, Order<Side::Sell, 101, 10, 1044>, Order<Side::Sell, 101, 10, 1045>, Order<Side::Sell, 101, 10, 1046>, Order<Side::Sell, 101, 10, 1047>, Order<Side::Sell, 101, 10, 1048>, Order<Side::Sell, 101, 10, 1049>, Order<Side::Sell, 101, 10, 1050>>>>;

// Incoming order that crosses
using IncomingOrder = Order<Side::Buy, 101, 50, 9999>;

using Result = Submit_t<InitialBook, IncomingOrder>;

static_assert(Result::book::valid);
static_assert(Result::trade_count > 0);

constexpr auto view = Result::runtime_view();

int main() {
    return 0;
}