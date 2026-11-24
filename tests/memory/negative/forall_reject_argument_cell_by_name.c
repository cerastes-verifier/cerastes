// expect: rejected cerastes-unsupported_annotation @ L20
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): a trailing argument that reads a cell by name (the global g) in a check is unsupported_annotation (the argument reads memory: question 248; M4.5e2)
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_m(int i, const int *a, int n, int m)
{
  return !(0 <= i && i < n) || a[i] >= m;
}

int g;

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_owns(g))
    CERASTES_ENSURES(cerastes_reads_range(a, n) && cerastes_owns(g))
{
  CERASTES_CHECK(cerastes_forall(ge_m, a, n, g));
  (void)a;
  (void)n;
}
