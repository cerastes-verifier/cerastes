// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/array_ptr_fill_one.c
// Pins: 0059 item 10: the callee kept a[0] (no ensures): the read of a[0] after f(a) has no permission
// Pre-registered from record 0059 items 4 and 10 (Part D), before the code
#include <cerastes.h>

void keep(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int use(void)
{
  int a[3] = {1, 2, 3};
  keep(a);
  return a[0];
}
