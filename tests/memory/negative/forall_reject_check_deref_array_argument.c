// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): `*p` of an array argument in a check is unsupported_annotation
// Pre-registered from record 0061 item 10 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool ge_first(int i, const int *p, int n)
{
  return !(0 <= i && i < n) || p[i] >= *p;
}

int run(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  CERASTES_CHECK(cerastes_forall(ge_first, a, 3));
  (void)a[0];
  return 0;
}
