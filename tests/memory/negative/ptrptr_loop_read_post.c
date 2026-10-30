// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/memory/ptrptr_loop_read.c
// Pins: 0040 item 2: the loop adds 7 per iteration, not 8
#include <cerastes.h>
#include <stddef.h>

int loop_read(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 8 * n)
{
  int x = 7;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  int s = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && s == 7 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s += **pp;
    i += 1;
  }
  CERASTES_CHECK(p == &x);
  return s;
}
