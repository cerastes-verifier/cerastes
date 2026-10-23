// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 2: a compound assignment reads its target, so it needs a value first; it then keeps one
#include <cerastes.h>

unsigned int bump(unsigned int a)
    CERASTES_ENSURES(r, r == a + 3)
{
  unsigned int x;
  x = a;
  x += 1u;
  x++;
  ++x;
  return x;
}

int bump_signed(int a)
    CERASTES_REQUIRES(-2147483647 <= a)
    CERASTES_ENSURES(r, r == a - 1)
{
  int x;
  x = a;
  x -= 1;
  return x;
}
