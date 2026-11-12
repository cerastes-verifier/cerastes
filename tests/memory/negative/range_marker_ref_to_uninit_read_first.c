// expect: error uninitialized @ L13
// mode: c23, c++20
// twin: tests/memory/range_marker_ref_to_uninit.c
// Pins: 0060 item 12: the callee reads p[0] before writing it, and its ref_to_uninit marker says nothing is initialized on entry: uninitialized at the read
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void wr(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3))
    CERASTES_ENSURES(cerastes_owns_range(p, 3))
{
  int x = p[0];
  p[0] = x;
}

int keeps(void)
{
  int a[3] = {1, 2, 3};
  wr(a);
  return a[1];
}
