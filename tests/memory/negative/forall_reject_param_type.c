// expect: rejected cerastes-forall_function @ L15
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3: every other parameter is a bool, an integer, an enumeration or a pointer to one of those or to a pointer: a double parameter is forall_function
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool wide(int i, const int *a, double d)
{
  return !(0 <= i) || a[i] >= 0 || d > 0;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(wide, a, 1.5))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
