#include "p03/core.hpp"
#include <iostream>
#include <type_traits>

template<class T> void describe(const char* name) {
    std::cout << name << " move_ctor=" << std::is_nothrow_move_constructible_v<T>
              << " move_assign=" << std::is_nothrow_move_assignable_v<T>
              << " adl_swap=" << std::is_nothrow_swappable_v<T> << '\n';
}
int main() {
    using namespace p03;
    describe<decltype(CoreState::orders)>("orders map");
    describe<PositionLedger>("PositionLedger");
    describe<ExecutionLedger>("ExecutionLedger");
    describe<FundsLedger>("FundsLedger");
    describe<decltype(CoreState::quarantine)>("quarantine vector");
    describe<decltype(CoreState::mark)>("mark optional");
    describe<CoreState>("CoreState");
}
