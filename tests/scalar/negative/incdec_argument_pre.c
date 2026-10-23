// expect: error precondition @ L18
// mode: c23, c++20
// twin: tests/scalar/incdec_argument.c
// Pins: 0020 item 1: x may be -1, which id() refuses
#include <cerastes.h>

int id(int v)
    CERASTES_REQUIRES(v >= 0)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int take(int x)
    CERASTES_REQUIRES(-1 <= x && x <= 1073741823)
    CERASTES_ENSURES(r, r == 2 * x + 1)
{
  int a = id(x++);
  return a + x;
}
