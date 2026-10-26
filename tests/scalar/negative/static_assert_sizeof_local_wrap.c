// expect: error overflow @ L12
// mode: c23, c++20
// twin: tests/scalar/static_assert_sizeof_local.c
// Pins: 0024 item 2: static_assert_sizeof_local.c with `&& 65536u * 65536u == 0u` added (the compilers accept: the product wraps to 0)
#include <cerastes.h>

int widths(int n)
    CERASTES_REQUIRES(n >= 0 && n <= 100)
    CERASTES_ENSURES(r, r == n + 4)
{
  int x = n;
  static_assert(sizeof(x) == sizeof(int) && 65536u * 65536u == 0u, "wraps");
  return x + (int)sizeof(x);
}
