// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/scalar/uninit_call_assign.c
// Pins: 0019 item 2: an argument is a read
#include <cerastes.h>

int ident(int v)
    CERASTES_ENSURES(r, r == v)
{
  return v;
}

int use_ident(void)
{
  int x;
  return ident(x);
}
