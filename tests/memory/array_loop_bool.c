// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0059 item 8: a loop over an array of bool: no range conjunct for bool (its flag and permission rows only)
// Pre-registered from record 0059 item 8 (Part C), before the code: verifies
#include <cerastes.h>

int count_true(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 3)
{
  bool a[3] = {true, false, true};
  int i = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_INVARIANT(0 <= c && c <= i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a[i]) {
      c = c + 1;
    }
    i = i + 1;
  }
  return c;
}
