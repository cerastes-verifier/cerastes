// expect: rejected cerastes-unsequenced_calls @ L18
// expect: frontend-warning assign_where_compare_meant @ L18
// mode: c23, c++20
// twin: tests/scalar/call_two_writes.c
// Pins: 0025 item 2: a call-free branch that holds a write is an item of the chain; here it depends on f(a) only and the division on g(b) only. EDG prints its #187 for the store in the && operand
#include <cerastes.h>

int f(int x)
    CERASTES_ENSURES(r, r == x);

int g(int y)
    CERASTES_REQUIRES(y != 0)
    CERASTES_ENSURES(r, r == y);

int test(int a, int b)
{
  int x = 0;
  int s = (f(a) > 0 && (x = 1)) + 100 / g(b);
  return s + x;
}
