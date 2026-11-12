// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/array_param_owns_bound.c
// Pins: 0060 item 15 (R1): the caller claims r == 8 where first returns the array's first element 7: postcondition at the return
// Pre-registered from record 0060 item 15 (Part E), before the code
#include <cerastes.h>

int first(int a[3])
    CERASTES_REQUIRES(cerastes_owns(*a))
    CERASTES_ENSURES(r, cerastes_owns(*a) && r == cerastes_old(*a))
{
  return a[0];
}

int use(void)
    CERASTES_ENSURES(r, r == 8)
{
  int x[3] = {7, 8, 9};
  return first(x);
}
