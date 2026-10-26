// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/scalar/uninit_guarded_comma.c
// Pins: 0025 item 1 (finding A): the write in the right operand of && follows the discarded guarded read of x
#include <cerastes.h>

int f(int a)
    CERASTES_ENSURES(r, r == a);

int h(int a)
{
  int x;
  int y = (((void)(f(a) > 0 && x > 0), 1) && ((x = 1) == 1));
  return y + x;
}
