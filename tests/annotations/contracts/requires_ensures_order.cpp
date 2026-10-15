// expect: golden
// expect: native-ok
// Pins: requires and ensures on one declaration, in either order, keep their
// source order in the listing.
#include <cerastes.h>

int a_first(int x) CERASTES_REQUIRES(x > 0) CERASTES_ENSURES(r, r > x);
int b_first(int x) CERASTES_ENSURES(r, r > x) CERASTES_REQUIRES(x > 0);
