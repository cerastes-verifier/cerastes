// expect: rejected cerastes-unsequenced_calls @ L19
// mode: c23, c++20
// twin: tests/scalar/call_two_writes.c
// Pins: 0025 item 2 (owner, planning): the snapshots of locals declared without an initializer carry uninitialized checks, which stay in the chain; with initialized locals (snapshots in call_two_writes.c) the same sum is accepted
#include <cerastes.h>

int capped(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == (x > 1000 ? 1000 : x));

int small(int y)
    CERASTES_REQUIRES(y >= 0)
    CERASTES_ENSURES(r, r == (y > 500 ? 500 : y));

int test(int a, int b)
    CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  int x, y;
  int s = (x = capped(a), x) + (y = small(b), y);
  return s;
}
