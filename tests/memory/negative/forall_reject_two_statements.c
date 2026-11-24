// expect: rejected cerastes-forall_function @ L16
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3: the body of f is exactly one return: a declaration before it is forall_function
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool two(int i, const int *a, int n)
{
  int x = n;
  return !(0 <= i && i < n) || a[i] >= x;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(two, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
