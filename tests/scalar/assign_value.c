// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0020 item 1: an assignment used as a value has the value stored, converted to the variable's type
#include <cerastes.h>

int id(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int chain(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a + 1)
{
  int y;
  int z = (y = a) + 1;
  return z + y;
}

int via_call(int a)
    CERASTES_REQUIRES(-1073741824 <= a && a <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * a)
{
  int y;
  int z = id(y = a);
  return z + y;
}
