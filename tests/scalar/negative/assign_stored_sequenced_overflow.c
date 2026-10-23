// expect: error overflow @ L18
// mode: c23, c++20
// twin: tests/scalar/assign_stored_sequenced.c
// Pins: 0020 item 4: the discarded x++ of x = (x++, 2) still overflows when x may be INT_MAX
#include <cerastes.h>

int g(int v)
    CERASTES_REQUIRES(v < 2147483647)
    CERASTES_ENSURES(r, r == v + 1)
{
  return v + 1;
}

int comma_store(int x)
    CERASTES_REQUIRES(x <= 2147483647)
    CERASTES_ENSURES(r, r == 2)
{
  x = (x++, 2);
  return x;
}

int call_store(int x)
    CERASTES_REQUIRES(x < 2147483647)
    CERASTES_ENSURES(r, r == x + 1)
{
  x = g(x++);
  return x;
}
