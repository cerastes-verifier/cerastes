// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/array_element_param.c
// Pins: 0059 item 4 (audit round 2, item 5b): a callee kept a[1]: the read of a[1] has no permission (the bound row passes, the permission row fails)
// Pre-registered from audit round 2 of record 0059 (item 5b)
#include <cerastes.h>

void keep(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int f(void)
{
  int a[2] = {1, 2};
  keep(&a[1]);
  return a[1];
}
