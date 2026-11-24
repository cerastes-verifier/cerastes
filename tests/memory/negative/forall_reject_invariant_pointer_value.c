// expect: rejected cerastes-unsupported_annotation @ L20
// mode: c23, c++20
// twin: tests/memory/forall_requires_read.c
// Pins: 0061 question 259 (fail-closed narrowing, lifted by M4.5e2): a pointer argument used as a value in E (p != q) in a loop invariant is unsupported_annotation
// Pre-registered from record 0061 item 10 (Part B), before the code
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool differ(int i, const int *p, const int *q, int n)
{
  return !(0 <= i && i < n) || (p != q && p[i] >= 0);
}

int run(void)
    CERASTES_ENSURES(r, r == 0)
{
  int a[3] = {1, 2, 3};
  int b[3] = {4, 5, 6};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && cerastes_forall(differ, a, b, i))
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = i + 1;
  }
  (void)a[0];
  (void)b[0];
  return 0;
}
