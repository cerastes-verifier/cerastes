// expect: error invariant-preserved @ L15
// mode: c23, c++20
// twin: tests/memory/loop_pick_pointer.c
// Pins: 0038 item 6: p == &x alone is not preserved: the loop may assign &y
#include <cerastes.h>

int pick_loop(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 1 || r == 2)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p == &x)
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
