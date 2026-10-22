// expect: error postcondition @ L24
// mode: c23, c++20
// twin: tests/scalar/const_global_init.c
// Pins: 0017 item 3: the exact ensures notices a wrong value of U
// use() is changed to what the wrong ensures would give, so the error is only the one in all_ones.
#include <cerastes.h>

const int G = 1 << 3;
const unsigned U = -1;
const signed char SC = (signed char)5;

int combine(int x)
    CERASTES_REQUIRES(0 <= x && x <= 100)
    CERASTES_ENSURES(r, r == x + 13)
{
  CERASTES_CHECK(0 <= x);
  CERASTES_CHECK(x <= 100);
  return x + G + SC;
}

unsigned all_ones(void)
    CERASTES_ENSURES(r, r == 4294967294u)
{
  return U;
}

int use(void)
    CERASTES_ENSURES(r, r == 131)
{
  int a = combine(0);
  int b = combine(100);
  int c = (int)(all_ones() - 4294967289u);
  return a + b + c;
}
