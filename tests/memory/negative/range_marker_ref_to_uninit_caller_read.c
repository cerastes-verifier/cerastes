// expect: error uninitialized @ L21
// mode: c23, c++20
// twin: tests/memory/range_marker_ref_to_uninit.c
// Pins: 0060 item 12 (S44 (h) ref_read_fail): the caller wrote a[0] only; ref_to_uninit does not promise the others: reading a[1] after the call is uninitialized
// Pre-registered from record 0060 item 12 (Part E), before the code
#include <cerastes.h>
#include <stddef.h>

void wr(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 3))
    CERASTES_ENSURES(cerastes_owns_range(p, 3))
{
  p[0] = 7;
}

int keeps(void)
{
  CERASTES_UNINIT int a[3];
  a[0] = 1;
  wr(a);
  return a[1];
}
