// expect: error invariant-preserved @ L17
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_write.c
// Pins: 0040 item 2: p == &x alone is not preserved: the loop stores &y
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
  CERASTES_INVARIANT(0 <= i && i <= n && p == &x)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *pp = &y;
    i += 1;
  }
  CERASTES_CHECK(*p == 7 || *p == 8);
  return x + y;
}
