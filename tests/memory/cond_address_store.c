// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0032 items 6, 9: a compound store through a conditional address: the write goes to x or to y, as the condition says
#include <cerastes.h>

int cond_store(int c)
    CERASTES_ENSURES(r, r == (c != 0 ? 11 : 21))
{
  int x = 10;
  int y = 20;
  *(c != 0 ? &x : &y) += 1;
  return c != 0 ? x : y;
}
