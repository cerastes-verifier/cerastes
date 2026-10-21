// expect: error invariant-preserved @ L14
// mode: c23, c++20
// twin: tests/scalar/continue_skip.c
// Pins: 0014 item 4: the continue path breaks the invariant, the normal path does not
// The continue branch sets c to 1, but the invariant needs c == 0 there; the normal path is unchanged.
#include <cerastes.h>

int count_after(int n, int start)
    CERASTES_REQUIRES(0 <= n && 0 <= start)
    CERASTES_ENSURES(r, (n <= start && r == 0) || (n > start && r == n - start))
{
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(i <= n && ((i <= start && c == 0) || (i > start && c == i - start)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    if (i <= start) {
      c = 1;
      continue;
    }
    c += 1;
  }
  return c;
}
