// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0014 item 4: a continue path goes to the loop test and re-checks the invariant
// Counts the values start + 1 .. n; the continue skips the count for the others.
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
      continue;
    }
    c += 1;
  }
  return c;
}
