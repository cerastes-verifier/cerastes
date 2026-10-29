// expect: rejected cerastes-unsupported_annotation @ L13
// mode: c23, c++20
// twin: tests/memory/loop_owns_fill.c
// Pins: 0038 item 10: a permission atom in a loop measure is rejected (never lifted)
#include <cerastes.h>

void measure_atom(int *p, int n)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(cerastes_owns(*p) ? n - i : 0)
  while (i < n) {
    i += 1;
  }
}
