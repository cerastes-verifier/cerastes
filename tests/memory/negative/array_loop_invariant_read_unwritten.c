// expect: error uninitialized @ L15
// mode: c23, c++20
// twin: tests/memory/array_loop_invariant_read.c
// Pins: 0059 item 8 (0038 item 6): the invariant reads a[i] (a[i] - a[i] == 0, which no value can falsify: the first run showed that a[i] >= 1 is also not preserved, a second real error at the clause) of an array whose elements but a[0] were never written: the flag obligation is uninitialized at the clause
// Pre-registered from record 0059 item 8 (Part C), before the code
#include <cerastes.h>

int first(void)
    CERASTES_ENSURES(r, r >= 1)
{
  int a[3];
  a[0] = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(a[i] - a[i] == 0)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    i = i + 1;
  }
  return a[0];
}
