// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/range_owning_literal.c
// Pins: 0060 item 13: f(&a[1]) hands over one element where owning(2) needs two: the shape row fails at the call (permission)
// Pre-registered from record 0060 item 13 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void f(CERASTES_OWNING_N(2) int *p)
{
  (void)p;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[2] = {1, 2};
  f(&a[1]);
  f(NULL);
  return 0;
}
