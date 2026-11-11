// expect: error out-of-bounds @ L20
// mode: c23, c++20
// twin: tests/memory/array_sizeof_bound.c
// Pins: 0059 item 11: sizeof(a) alone is the size in bytes (12), not the count: used as the loop bound it indexes past the end (the postcondition is widened to r <= 36 so that the exit state is not a second error)
// Pre-registered from record 0059 item 11 (Part E), before the code
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 36)
{
  int a[3] = {1, 2, 3};
  int n = (int)sizeof(a);
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_INVARIANT(0 <= s)
  CERASTES_INVARIANT(s <= 3 * i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    s = s + a[i];
    i = i + 1;
  }
  return s;
}
