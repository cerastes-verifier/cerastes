// expect: error overflow @ L10
// twin: tests/scalar/static_assert_local_constexpr.cpp
// Pins: 0024 item 2: static_assert_local_constexpr.cpp with k = 65536u: the product wraps; k's own initializer is in range, so there is exactly one error, at the assertion
#include <cerastes.h>

int square_of_k(void)
    CERASTES_ENSURES(r, r == 7)
{
  constexpr unsigned k = 65536u;
  static_assert(k * k == 0u, "wraps");
  return 7;
}
