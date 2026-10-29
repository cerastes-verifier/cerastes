// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3 (audit B4): a cell declared after the loop in the enclosing block is allocated at the block's entry, so it is one of the loop's places and a return inside the loop exits it
#include <cerastes.h>

int later_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      return 3;
    }
    i += 1;
  }
  int x = 3;
  int *p = &x;
  return *p;
}
