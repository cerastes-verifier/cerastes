// expect: error overflow @ L18
// mode: c23, c++20
// twin: tests/scalar/call_in_expression.c
// Pins: 0010 item 3: the nested calls promise up to 2000000000, and adding 2000000000 overflows
#include <cerastes.h>

int bounded(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r >= 0 && r <= 2000000000)
{
  return x > 1000 ? 1000 : x;
}

int chain(int a)
    CERASTES_REQUIRES(a >= 0)
    CERASTES_ENSURES(r, r >= 0)
{
  return bounded(bounded(a)) + 2000000000;
}
