// expect: rejected cerastes-forall_function @ L17
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 item 3: the bound variable has an integer type other than the enumerations: an enumeration binder is forall_function
// Pre-registered from record 0061 item 3 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

enum Color { RED, GREEN };

CERASTES_PURE bool shade(enum Color i, const int *a, int n)
{
  return a[0] >= n || i == RED;
}

void use(const int *a, int n)
    CERASTES_REQUIRES(cerastes_reads_range(a, n) && cerastes_forall(shade, a, n))
    CERASTES_ENSURES(cerastes_reads_range(a, n))
{
  (void)a;
  (void)n;
}
