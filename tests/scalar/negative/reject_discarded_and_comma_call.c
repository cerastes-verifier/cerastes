// expect: rejected cerastes-value_not_used @ L15
// mode: c23, c++20
// twin: tests/scalar/call_discarded_cond.c
// Pins: 0020 item 5 (P37): a discarded && whose right operand is a comma that ends in a constant
#include <cerastes.h>

int g(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int f(int a, int x)
{
  a && (g(x), 1);
  return a;
}
