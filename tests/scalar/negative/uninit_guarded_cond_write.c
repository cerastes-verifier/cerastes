// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/scalar/uninit_guarded_comma.c
// Pins: 0025 item 1 (finding A): the write of x follows the read in the arm of a discarded ?:
#include <cerastes.h>

int f(int a)
    CERASTES_ENSURES(r, r == a);

int h(int a)
{
  int x;
  int y = (x = ((void)(f(a) > 0 ? x : 0), 2));
  return y + x;
}
