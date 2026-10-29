// expect: error lifetime @ L20
// mode: c23, c++20
// twin: tests/memory/loop_return_param_cell.c
// Pins: 0038 items 3 and 7: the callee kept the copy of the parameter, so the scope exit at the return inside the loop fails
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int pass_through(int a, int n)
    CERASTES_REQUIRES(0 <= a && a <= 100 && 0 <= n && n <= 100)
{
  int *pa = &a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (i == 5) {
      dispose(pa);
      return i;
    }
    i += 1;
  }
  return *pa;
}
