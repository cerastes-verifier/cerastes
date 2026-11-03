// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 9, 12: a written atom on owns(*s.q) of a by-value parameter's pointer member
#include <cerastes.h>
#include <stddef.h>

struct P {
  int *q;
};

void walk(struct P s, int n)
    CERASTES_REQUIRES(cerastes_owns(*s.q) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*s.q))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && cerastes_owns(*s.q))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *s.q = i;
    i += 1;
  }
}
