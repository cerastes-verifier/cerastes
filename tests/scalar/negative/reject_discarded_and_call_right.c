// expect: rejected cerastes-value_not_used @ L15
// mode: c23, c++20
// twin: tests/scalar/call_discarded_cond.c
// Pins: 0020 item 5 (P37): a discarded && whose right operand is a call: the result of the && is unused
#include <cerastes.h>

int g(int x)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}

int f(int a, int x)
{
  a && g(x);
  return a;
}
