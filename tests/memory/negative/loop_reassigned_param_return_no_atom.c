// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/loop_reassigned_param_return.c
// Pins: 0040 item 5 (was 0038 Consequences, audit N2): without a written atom the return inside the loop cannot give back the place of a parameter the function assigns
#include <cerastes.h>

int moved(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(r, cerastes_owns(*p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return i;
    }
    i += 1;
  }
  return n;
}
