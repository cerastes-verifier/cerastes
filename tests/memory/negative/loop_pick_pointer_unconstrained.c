// expect: error null-dereference @ L21
// expect: error permission @ L21
// expect: error null-dereference @ L28
// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/loop_pick_pointer.c
// Pins: 0038 item 6: without a clause on p the pointer the loop assigns is any pointer at the loop head and after the loop: at each use *p may be null and, the null check assumed, points to no held object
#include <cerastes.h>

int pick_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1 || r == 2)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*p == 1) {
      i += 1;
    } else {
      i += 1;
    }
    p = (i % 2 == 0) ? &y : &x;
  }
  return *p;
}
