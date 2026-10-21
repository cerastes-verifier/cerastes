// expect: error termination @ L15
// mode: c23, c++20
// twin: tests/scalar/for_decl_jump.c
// Pins: 0014 item 4: a measure that the continue path does not decrease
// s does not change on the continue path (c == 3), so n - s does not decrease there.
#include <cerastes.h>

int count_for(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, (n < 3 && r == n) || (3 <= n && n <= 6 && r == n - 1) || (n >= 7 && r == 6))
{
  int c = 0;
  int s = 0;
  CERASTES_INVARIANT(c <= n && c <= 6 && ((c < 3 && s == c) || (c >= 3 && s == c - 1)))
  CERASTES_DECREASES(n - s)
  for (int k = 0; c < n; k = c) {
    c += 1 + (k - k);
    if (c == 3) {
      continue;
    }
    s += 1;
    if (c == 7) {
      break;
    }
  }
  return s;
}
