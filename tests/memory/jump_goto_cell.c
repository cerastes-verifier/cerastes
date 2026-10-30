// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 7: an outward goto leaves a block that holds a cell (the shape that 0032 item 10 rejected); the scope exit is at the goto
#include <cerastes.h>

int goto_cell(int a)
    CERASTES_REQUIRES(-100 <= a && a <= 100)
    CERASTES_ENSURES(r, (a > 5 && r == 0) || (a <= 5 && r == a))
{
  int total = 0;
  {
    int x = a;
    int *p = &x;
    if (*p > 5) {
      goto done;
    }
    total = x;
  }
done:
  return total;
}
