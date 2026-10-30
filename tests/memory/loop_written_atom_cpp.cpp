// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0040 item 5: a written atom in the C++ spelling, for a parameter the function assigns
#include <cerastes.h>
#include <stddef.h>

void touch(int *p, int n)
    CERASTES_REQUIRES(cerastes::owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes::owns(*p))
{
  int *q = p;
  p = q;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes::owns(*p))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
}
