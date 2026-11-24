// expect: rejected cerastes-unsupported_expression @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 4 (audit round 1 S13): & of a parameter of f has no meaning: unsupported_expression
// Pre-registered from record 0061 item 4 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool addr_n(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || (&n != NULL && a[i] >= 0);
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(addr_n, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
