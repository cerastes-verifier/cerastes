// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3: a write to another cell by name does not reach x (only a store through a pointer to its field type does), so x keeps its value
#include <cerastes.h>

int other_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  int y = 0;
  int *p = &x;
  int *q = &y;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    y = i;
    i += 1;
  }
  return *p + (*q - *q);
}
