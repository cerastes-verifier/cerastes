// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 9, 10 (question 105): owns(*s.q) of a by-value parameter's pointer member is an automatic loop place directly through its leaf formal
#include <cerastes.h>
#include <stddef.h>

struct P {
  int *q;
};

void bump_n(struct P s, int n)
    CERASTES_REQUIRES(cerastes_owns(*s.q) && *s.q == 0 && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*s.q) && *s.q == n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *s.q == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *s.q += 1;
    i += 1;
  }
}
