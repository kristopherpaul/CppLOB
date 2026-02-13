#include <compile_time/level.hpp>
#include <compile_time/order.hpp>
#include <compile_time/side_book.hpp>
#include <compile_time/type_list.hpp>
#include <type_traits>

using namespace lob::compile_time;
using namespace lob::spec;

template <class T>
struct IsInt : std::bool_constant<std::is_same_v<T, int>> {};

static_assert(TypeList<>::size == 0);
static_assert(list_size_v<TypeList<int, double, char>> == 3);
static_assert(std::is_same_v<PushFront_t<int, TypeList<double>>, TypeList<int, double>>);
static_assert(std::is_same_v<PushBack_t<TypeList<int>, double>, TypeList<int, double>>);
static_assert(std::is_same_v<PopFront_t<TypeList<int, double>>, TypeList<double>>);
static_assert(std::is_same_v<Concat_t<TypeList<int>, TypeList<double>>, TypeList<int, double>>);
static_assert(std::is_same_v<Filter_t<TypeList<int, double, int>, IsInt>, TypeList<int, int>>);
static_assert(std::is_same_v<At_t<1, TypeList<int, double>>, double>);
static_assert(std::is_same_v<Front_t<TypeList<int, double>>, int>);
static_assert(std::is_same_v<Back_t<TypeList<int, double>>, double>);
static_assert(find_if_v<TypeList<double, char, int>, IsInt> == 2);

using Buy1 = Order<Side::Buy, 100, 10, 1>;
using Buy2 = Order<Side::Buy, 100, 20, 2>;
using Level = PriceLevel<100, Buy1, Buy2>;
static_assert(ValidOrder<Buy1>);
static_assert(Level::total_quantity == 30);
static_assert(Level::best_order::id == 1);
static_assert(LevelPushBack_t<Level, Order<Side::Buy, 100, 5, 3>>::order_count == 3);
static_assert(LevelPopFrontN_t<Level, 1>::best_order::id == 2);
static_assert(EmptyPriceLevel<100>::is_empty);

using Bid100 = AddOrderToBidBook_t<SideBook<>, Buy1>;
using Bid101 = AddOrderToBidBook_t<Bid100, Order<Side::Buy, 101, 5, 4>>;
using Bid99 = AddOrderToBidBook_t<Bid101, Order<Side::Buy, 99, 5, 5>>;
static_assert(Bid99::best_price == 101);
static_assert(Bid99::level_count == 3);
static_assert(At_t<0, typename Bid99::levels>::price == 101);

using Ask100 = AddOrderToAskBook_t<SideBook<>, Order<Side::Sell, 100, 5, 6>>;
using Ask99 = AddOrderToAskBook_t<Ask100, Order<Side::Sell, 99, 5, 7>>;
static_assert(Ask99::best_price == 99);

int main() {
    return 0;
}
