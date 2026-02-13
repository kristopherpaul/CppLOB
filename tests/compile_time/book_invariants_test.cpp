#include <compile_time/match.hpp>
#include <type_traits>

using namespace lob::compile_time;
using namespace lob::spec;

using Empty = EmptyBook;
using Buy = Order<Side::Buy, 100, 10, 1>;
using Sell = Order<Side::Sell, 105, 10, 2>;
using WithBuy = AddOrder_t<Empty, Buy>;
using WithBoth = AddOrder_t<WithBuy, Sell>;

static_assert(Empty::valid);
static_assert(Empty::is_empty);
static_assert(!Empty::best_bid().has_value());
static_assert(WithBoth::valid);
static_assert(WithBoth::best_bid() == 100);
static_assert(WithBoth::best_ask() == 105);
static_assert(WithBoth::total_level_count == 2);

using InvalidSide = Book<SideBook<PriceLevel<100, Sell>>, SideBook<>>;
using Crossed = Book<SideBook<PriceLevel<105, Buy>>, SideBook<PriceLevel<100, Sell>>>;
using Duplicate = Book<SideBook<PriceLevel<100, Buy, Buy>>, SideBook<>>;
static_assert(!InvalidSide::valid);
static_assert(!Crossed::valid);
static_assert(!Duplicate::valid);

using First = AddOrder_t<Empty, Order<Side::Buy, 99, 1, 10>>;
using Second = AddOrder_t<First, Order<Side::Buy, 99, 1, 11>>;
using Third = AddOrder_t<Second, Order<Side::Buy, 99, 1, 12>>;
using CancelMiddle = CancelOrder_t<Third, 11>;
static_assert(CancelMiddle::bids::best_level::order_count == 2);
static_assert(At_t<0, typename CancelMiddle::bids::best_level::orders>::id == 10);
static_assert(At_t<1, typename CancelMiddle::bids::best_level::orders>::id == 12);
using CancelAll = CancelOrder_t<CancelMiddle, 10>;
using EmptyAfterCancel = CancelOrder_t<CancelAll, 12>;
static_assert(EmptyAfterCancel::is_empty);
static_assert(CancelOrder_t<WithBoth, 999>::valid);
static_assert(CancelOrder_t<WithBoth, 999>::best_bid() == 100);

int main() {
    return 0;
}
