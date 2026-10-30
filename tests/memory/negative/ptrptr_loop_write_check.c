// expect: error check @ L23
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_write.c
// Pins: 0040 item 2: the invariant pins p to x or y but the check claims *p == 7 only: the loop may have stored &y
#include <cerastes.h>
#include <stddef.h>

int loop_write(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 15)
{
  int x = 7;
  int y = 8;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (p == &x || p == &y))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pp = &y;
    i += 1;
  }
  CERASTES_CHECK(*p == 7);
  return x + y;
}
