// expect: error conversion @ L12
// mode: c23, c++20
// twin: tests/scalar/char_literal_unreachable.c
// Pins: 0024 item 5: char_literal_unreachable.c with the requires widened to x <= 11: x == 11 reaches the wrapped literal
#include <cerastes.h>

int guarded(int x)
    CERASTES_REQUIRES(x >= 0 && x <= 11)
    CERASTES_ENSURES(r, r == x)
{
  if (x > 10) {
    char c = '\xff';
    return c;
  }
  return x;
}

int use(void)
    CERASTES_ENSURES(r, r == 10)
{
  return guarded(10);
}
