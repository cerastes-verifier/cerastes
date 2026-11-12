// expect: error postcondition @ L24
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/range_marker_strict_init.c
// Pins: 0060 item 12: the caller claims r == 2 where the filler promises a[0] == 1 (under --strict-init): postcondition at the return
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void fill3(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3))
    CERASTES_ENSURES(cerastes_owns_range(p, 3) && p[0] == 1)
{
  p[0] = 1;
  p[1] = 2;
  p[2] = 3;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  CERASTES_UNINIT int a[3];
  fill3(a);
  return a[0] + (a[2] - a[2]);
}
