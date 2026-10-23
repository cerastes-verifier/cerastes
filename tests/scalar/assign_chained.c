// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: a = b = c stores the value of each assignment in the one to its left
#include <cerastes.h>

int chain(int a)
    CERASTES_REQUIRES(-715827882 <= a && a <= 715827882)
    CERASTES_ENSURES(r, r == 3 * a)
{
  int x;
  int y;
  int z;
  x = y = z = a;
  return x + y + z;
}

int init_chain(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a)
{
  int p;
  int q = p = a;
  return p + q;
}
