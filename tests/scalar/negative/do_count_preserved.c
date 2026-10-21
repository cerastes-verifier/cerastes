// expect: error invariant-preserved @ L13
// mode: c23, c++20
// twin: tests/scalar/do_count.c
// Pins: 0014 item 5: the invariant must hold again at the start of the next iteration
// For n > 3 the iteration that leaves i == 3 starts another one.
#include <cerastes.h>

int count_up(int n)
    CERASTES_REQUIRES(1 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  CERASTES_INVARIANT(i < n && i < 3)
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
  } while (i < n);
  return i;
}
