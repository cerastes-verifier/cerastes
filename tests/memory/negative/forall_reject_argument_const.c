// expect: rejected cerastes-forall_argument_mismatch @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3: a `const int *` argument for an `int *` parameter is forall_argument_mismatch (the converse, `int *` for `const int *`, is allowed: forall_argument_qualifiers.c)
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg_mut(int i, int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(nonneg_mut, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
