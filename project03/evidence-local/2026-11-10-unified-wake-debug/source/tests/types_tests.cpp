#include "fixture.hpp"
#include <limits>
#include <type_traits>
using namespace fixture;

TEST_CASE(A01_identity_types_and_exhaustion) {
    static_assert(!std::is_same_v<OrderId,CommandId>);
    static_assert(!std::is_convertible_v<OrderId,ReservationId>);
    IdSequence<OrderTag> ids(17,std::numeric_limits<std::uint64_t>::max());
    const auto last=ids.take(); CHECK(last && last->instance==17); CHECK(!ids.take());
    CHECK(!OrderId{}.valid()); CHECK_THROWS(IdSequence<OrderTag>(0),std::invalid_argument);
}
TEST_CASE(A01_dates_validate_gregorian) {
    CHECK(TradingDay{20240229}.valid()); CHECK(!TradingDay{20260229}.valid());
    CHECK(!TradingDay{20261301}.valid()); CHECK(!TradingDay{0}.valid());
}
TEST_CASE(A01_calendar_night_mapping_and_breaks) {
    const auto c=config();
    CHECK_EQ(c.calendar.validate({{20261104},21*3600+1,day},day),Error::None);
    CHECK_EQ(c.calendar.validate({{20261104},21*3600+1,{20261104}},day),Error::WrongTradingDay);
    CHECK_EQ(c.calendar.validate({day,10*3600+20*60,day},day),Error::ClosedSession);
    CHECK_EQ(c.calendar.validate({day,15*3600,day},day),Error::ClosedSession);
    CHECK_EQ(c.calendar.validate({{20261107},9*3600,day},day),Error::ClosedSession);
    CHECK_EQ(c.calendar.validate({day,86400,day},day),Error::InvalidInput);
}
TEST_CASE(A01_bad_calendar_configuration_rejected) {
    CHECK_THROWS((TradingCalendar{"fixture",{{day,10,20,day},{day,19,30,day}}}),std::invalid_argument);
    CHECK_THROWS((TradingCalendar{"fixture",{{day,20,10,day}}}),std::invalid_argument);
}
TEST_CASE(A01_contract_tick_quantity_and_day_rules) {
    auto c=config(); c.contract.tick_step=2; c.contract.lot_step=2; c.contract.min_lots=2;
    c.contract.check_configuration(); auto i=intent(); i.quantity=2; i.price=500;
    CHECK_EQ(c.contract.validate(i),Error::None);
    i.price=499; CHECK_EQ(c.contract.validate(i),Error::PriceRule); i.price=500;
    i.quantity=3; CHECK_EQ(c.contract.validate(i),Error::QuantityRule); i.quantity=2;
    i.day=next_day; CHECK_EQ(c.contract.validate(i),Error::WrongTradingDay); i.day=day;
    i.instrument="RB"; CHECK_EQ(c.contract.validate(i),Error::NoContract);
    i.instrument="rb_MAIN"; CHECK_EQ(c.contract.validate(i),Error::NoContract);
    i.instrument="rb_FIXTURE"; i.price=1001; CHECK_EQ(c.contract.validate(i),Error::PriceRule);
}
TEST_CASE(A01_unsupported_real_profile_cannot_boot) {
    auto c=config(); c.contract.synthetic_fixture=false;
    CHECK_THROWS(CoreLoop{c},std::invalid_argument);
    c=config(); c.contract.instrument="RB"; CHECK_THROWS(CoreLoop{c},std::invalid_argument);
}
TEST_CASE(A01_checked_arithmetic_boundaries) {
    const auto hi=std::numeric_limits<std::int64_t>::max();
    const auto lo=std::numeric_limits<std::int64_t>::min();
    CHECK_EQ(checked_add(hi,-1),hi-1);
    CHECK_THROWS(checked_add(hi,1),std::overflow_error);
    CHECK_THROWS(checked_add(lo,-1),std::overflow_error);
    CHECK_THROWS(checked_mul_nonnegative(hi,2),std::overflow_error);
    CHECK_THROWS(checked_mul_nonnegative(-1,2),std::invalid_argument);
    CHECK_EQ(checked_mul_nonnegative(0,hi),0);
}
TEST_CASE(P01_six_position_effects) {
    const std::array<std::pair<Side,Offset>,6> actions{{{Side::Buy,Offset::Open},{Side::Sell,Offset::Open},
        {Side::Sell,Offset::CloseToday},{Side::Sell,Offset::CloseYesterday},
        {Side::Buy,Offset::CloseToday},{Side::Buy,Offset::CloseYesterday}}};
    const std::array<std::array<Lots,4>,6> expected{{{6,6,7,8},{5,6,8,8},{4,6,7,8},{5,5,7,8},{5,6,6,8},{5,6,7,7}}};
    for(std::size_t n=0;n<actions.size();++n) {
        PositionLedger p(config({5,6,7,8}).anchor);
        p.book(actions[n].first,actions[n].second,1); CHECK_EQ(p.booked(),expected[n]); CHECK(!p.deficit());
    }
}
TEST_CASE(P01_anchor_is_not_mutated_by_booking) {
    PositionLedger p(config({2,3,4,5}).anchor); const auto original=p.anchor();
    p.book(Side::Buy,Offset::Open,2); CHECK_EQ(p.anchor(),original);
    p.replace_freezes({1,0,0,0}); CHECK_EQ(p.available(Bucket::LongToday),3);
    CHECK_THROWS(p.book(Side::Buy,Offset::Open,0),std::invalid_argument);
    CHECK_EQ(p.booked()[0],4);
}
TEST_CASE(P01_invalid_side_offset_and_bucket_fail_without_mutation) {
    PositionLedger p(config({1,2,3,4}).anchor); const auto before=p.booked();
    CHECK_THROWS(p.book(static_cast<Side>(99),Offset::Open,1),std::invalid_argument);
    CHECK_THROWS(p.book(Side::Buy,static_cast<Offset>(99),1),std::invalid_argument);
    CHECK_THROWS(p.available(static_cast<Bucket>(99)),std::invalid_argument);
    CHECK_EQ(p.booked(),before);
}
