// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23
// c-only: [static N] in a parameter is C only (a front-end error in C++)
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3 (audit round 1 S13, 0059 item 1): an [static N] parameter of f is rejected with the tag it has on any function
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool stat(int i, const int b[static 3])
{
  return !(0 <= i && i < 3) || b[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(stat, a))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
