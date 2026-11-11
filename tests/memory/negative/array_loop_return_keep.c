// expect: error lifetime @ L21
// mode: c23, c++20
// twin: tests/memory/array_loop_return.c
// Pins: 0059 item 8 and 0032 item 8: a callee keeps a[2] just before the return inside the loop: the scope exit at that return reports lifetime (the other path to the back edge is untouched)
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int find(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 3)
{
  int a[3] = {5, 1, 2};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i] == 1) {
      dispose(&a[2]);
      return i;
    }
    i = i + 1;
  }
  return 3;
}
