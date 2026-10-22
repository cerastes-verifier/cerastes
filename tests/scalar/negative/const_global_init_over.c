// expect: error overflow @ L7
// c-only: C++ makes this initializer a dynamic initialization and rejects it (reject_global_dynamic.cpp); C23 folds it
// twin: tests/scalar/const_global_init.c
// Pins: 0017 items 4 and 8: C23 folds the initializer (EDG: -2147483648) and the sum overflows
#include <cerastes.h>

const int G = 2147483647 + 1;
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
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return U;
}

int use(void)
    CERASTES_ENSURES(r, r == 131)
{
  int a = combine(0);
  int b = combine(100);
  int c = (int)(all_ones() - 4294967290u);
  return a + b + c;
}
