// expect: rejected cerastes-forall_argument_mismatch @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (P80 (e)): a char argument for an int parameter is forall_argument_mismatch (the written type is char, under C's generated promotion)
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_m(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] >= m;
}

void use(const int *a, int n, char x)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(ge_m, a, n, x))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
  (void)x;
}
