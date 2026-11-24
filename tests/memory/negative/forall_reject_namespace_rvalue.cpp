// expect: rejected cerastes-unsupported_annotation_expression @ L14
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 2 (cut 3, P80 (b)): an rvalue trailing argument (n + 1) arrives as enk_temp_init, which the front end's annotation walker refuses (unsupported_annotation_expression), the diagnostic that decided cut 3
// Pre-registered from record 0061 item 2 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool nonneg(int i, const int *a, int n)
{
  return !(0 <= i && i < n) || a[i] >= 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes::forall(nonneg, a, n + 1))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
