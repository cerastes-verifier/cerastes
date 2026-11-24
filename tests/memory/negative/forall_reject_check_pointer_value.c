// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): a pointer argument used as a value in E (p != q) in a check is unsupported_annotation
// Pre-registered from record 0061 item 10 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool differ(int i, const int *p, const int *q, int n)
{
  return !(0 <= i && i < n) || (p != q && p[i] >= 0);
}

void use(const int *p, const int *q, int n)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && cerastes_reads_range(q, n))
    CERASTES_ENSURES(cerastes_reads_range(p, n) && cerastes_reads_range(q, n))
{
  CERASTES_CHECK(cerastes_forall(differ, p, q, n));
  (void)p;
  (void)q;
  (void)n;
}
