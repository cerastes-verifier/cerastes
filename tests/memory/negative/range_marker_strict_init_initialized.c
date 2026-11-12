// expect: rejected cerastes-uninit_ref_initialized @ L23
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/range_marker_strict_init.c
// Pins: 0060 item 12 (0042 item 8): under --strict-init an array that has an initializer is not an uninit array, so passing it to a must_init parameter is rejected at the call (uninit_ref_initialized)
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
    CERASTES_ENSURES(r, r == 1)
{
  int a[3] = {0, 0, 0};
  fill3(a);
  return a[0] + (a[2] - a[2]);
}
