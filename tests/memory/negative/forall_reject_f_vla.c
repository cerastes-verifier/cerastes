// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23
// c-only: a variable length array parameter is C only (EDG's #411 rejects it in C++)
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (audit round 1 S13, 0060 item 15): a VLA parameter of f is rejected with the tag it has on any function
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool vla(int i, int m, int b[m])
{
  return !(0 <= i && i < m) || b[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(vla, n, a))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
