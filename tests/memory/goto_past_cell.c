// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 item 4 (allocation at block entry) and 0019 item 6: a goto past a cell's declaration finds the cell allocated and uninitialized
#include <cerastes.h>

int skip_cell(int a)
    CERASTES_ENSURES(r, r == (a > 5 ? 7 : 10))
{
  int y = 0;
  if (a > 5) {
    goto done;
  }
  int x;
  int *q;
  q = &x;
  *q = 3;
  y = x;
done:
  x = 7;
  return y + x;
}
