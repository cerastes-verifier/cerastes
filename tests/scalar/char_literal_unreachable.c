// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0024 item 5: the conversion of a wrapped literal is an obligation on its path (like an overflow in a branch), not a property of the program text
#include <cerastes.h>

int guarded(int x)
    CERASTES_REQUIRES(x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 10) {
    char c = '\xff';
    return c;
  }
  return x;
}

int use_zero(void)
    CERASTES_ENSURES(r, r == 0)
{
  return guarded(0);
}

int use(void)
    CERASTES_ENSURES(r, r == 10)
{
  return guarded(10);
}
