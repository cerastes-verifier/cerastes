// expect: rejected cerastes-unsupported_expression @ L11
// mode: c23, c++20
// twin: tests/memory/array_loop_invariant_read.c
// Pins: 0059 item 8: an element of a pointer in a loop invariant or measure is rejected; the text names e1b (lifted by M4.5e1b)
#include <cerastes.h>

int count(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 10 && *p >= 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && p[0] >= 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return i;
}
