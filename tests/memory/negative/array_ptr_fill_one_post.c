// expect: error postcondition @ L20
// mode: c23, c++20
// twin: tests/memory/array_ptr_fill_one.c
// Pins: 0059 item 10: the callee wrote 5 into a[0], so r == 1 fails
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

void fill_one(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 5)
{
  *p = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {1, 2, 3};
  fill_one(a);
  return a[0];
}
