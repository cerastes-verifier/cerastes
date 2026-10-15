// expect: golden
// expect: native-ok
// Pins: ensures(p) without a result name on functions whose return type is
// deduced (auto, decltype(auto)) or trailing; the contract goes between the
// parameter list and the trailing return type. Twin of the ensures_binder_*
// negatives (a result name is an error in all three).
#include <cerastes.h>

auto next_auto(int x) CERASTES_ENSURES(x < 1000) { return x + 1; }

decltype(auto) next_dt(int x) CERASTES_ENSURES(x < 1000) { return x + 1; }

auto next_tr(int x) CERASTES_ENSURES(x < 1000) -> int { return x + 1; }
