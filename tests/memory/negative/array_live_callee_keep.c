// expect: error lifetime @ L16
// mode: c23, c++20
// twin: tests/memory/array_live_callee.c
// Pins: 0059 item 6 (0037 item 13): the callee takes the liveness of the array's block and does not give it back: the unmarked liveness row at the scope exit fails, reported at the return as `the address of a may still be in use` (audit round 2, item 5a)
// Pre-registered from audit round 2 of record 0059 (item 5a)
#include <cerastes.h>

void hold(int *p)
    CERASTES_REQUIRES(cerastes_live(p));

int f(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[2] = {1, 2};
  hold(&a[1]);
  return a[0];
}
