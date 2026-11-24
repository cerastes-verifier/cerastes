// expect: rejected cerastes-unsupported_annotation @ L19
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): `*p` (a read the binder does not index) of an array argument in a loop invariant is unsupported_annotation
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
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(ge_first, a, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  (void)a[0];
  return 0;
}
