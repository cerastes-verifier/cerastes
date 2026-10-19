// expect: error overflow @ L17
// mode: c23, c++20
// twin: tests/scalar/long_arith.c
// Pins: 0010 item 3: long addition may overflow
#include <cerastes.h>

long widen_add(int a, int b)
    CERASTES_ENSURES(r, r == (long)a + b)
{
  return (long)a + b;
}

long add_long(long a, long b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
    CERASTES_ENSURES(r, r >= a)
{
  return a + b;
}
