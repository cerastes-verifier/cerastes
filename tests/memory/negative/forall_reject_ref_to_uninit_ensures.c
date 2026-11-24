// expect: rejected cerastes-unsupported_annotation @ L16
// mode: c23, c++20
// twin: tests/memory/forall_must_init_ensures.c
// Pins: 0061 item 4 (0042 item 6): a read of a ref_to_uninit parameter under the binder in its ensures is unsupported_annotation (only must_init promises the flags)
// Pre-registered from record 0061 item 4 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int i, const int *p, int n)
{
  return !(0 <= i && i < n) || p[i] == 0;
}

void use(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns_range(p, 2))
    CERASTES_ENSURES(cerastes_owns_range(p, 2) && cerastes_forall(zero_at, p, 2))
{
  p[0] = 0;
  p[1] = 0;
}
