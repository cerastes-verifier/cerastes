// expect: rejected cerastes-value_not_used @ L10
// mode: c23, c++20
// twin: tests/scalar/call_discarded_cond.c
// Pins: 0020 item 5 (P37): a discarded && whose left operand is an increment
#include <cerastes.h>

int f(int a, int x)
    CERASTES_REQUIRES(x < 2147483647)
{
  x++ && a;
  return x;
}
