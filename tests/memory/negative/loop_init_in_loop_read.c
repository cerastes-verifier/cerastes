// expect: error uninitialized @ L19
// mode: c23, c++20
// twin: tests/memory/loop_init_in_loop.c
// Pins: 0038 Consequences (S28 F2): the flag of a cell the loop first initializes is only known to stay true, so a read after the loop without the value in the invariant is uninitialized
#include <cerastes.h>

int init_in_loop(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  int x;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i + 1;
    i += 1;
  }
  return *p;
}
