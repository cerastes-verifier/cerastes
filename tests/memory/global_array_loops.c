// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 10: in main an array global is an automatic place of a loop: a read-only sum loop keeps the values (s <= 3 * i holds from the initial list) and a fill loop keeps the permission, the flags and the range, not the values
// Pre-registered from record 0063 item 10 (Part B), before any run: verifies
#include <cerastes.h>

static int g[3] = {1, 2, 3};

int main(void)
{
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && 0 <= s && s <= 3 * i)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    s += g[i];
    i += 1;
  }
  CERASTES_CHECK(s <= 9);
  i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    g[i] = i;
    i += 1;
  }
  return s - s;
}
