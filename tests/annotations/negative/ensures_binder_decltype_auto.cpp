// expect: frontend-error attr_result_name_deduced_type @ L10
// expect: native-ok
// twin: tests/annotations/contracts/deduced_return.cpp
// Mutation of the twin: a result name on a decltype(auto) function.
// Pins: only this diagnostic is reported (a one-error file, no incidental noise).
#include <cerastes.h>

auto next_auto(int x) CERASTES_ENSURES(x < 1000) { return x + 1; }

decltype(auto) next_dt(int x) CERASTES_ENSURES(r, r < 1000) { return x + 1; }

auto next_tr(int x) CERASTES_ENSURES(x < 1000) -> int { return x + 1; }
