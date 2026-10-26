// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/scalar/uninit_guarded_comma.c
// Pins: 0025 item 1 (finding A): the write of x follows the read in the guarded operand (the comma drops the call that guards it); when f(a) > 0 the read sees an uninitialized x
#include <cerastes.h>

int g(int v)
    CERASTES_ENSURES(r, r == v);

int f(int a)
    CERASTES_ENSURES(r, r == a);

int h(int a)
{
  int x;
  int y = (x = (g(f(a) > 0 && x > 0), 2));
  return y + x;
}
