// expect: rejected cerastes-unsupported_annotation @ L15
// mode: c23, c++20
// twin: tests/memory/forall_must_init_ensures.c
// Pins: 0061 item 4 (0042 item 6): a read of a must_init parameter under the binder in a requires is unsupported_annotation (its flag is not promised on entry)
// Pre-registered from record 0061 item 4 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool zero_at(int i, const int *p, int n)
{
  return !(0 <= i && i < n) || p[i] == 0;
}

void use(int *p CERASTES_MUST_INIT, int n)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_forall(zero_at, p, n))
    CERASTES_ENSURES(cerastes_owns_range(p, n))
{
  p[0] = 0;
}
