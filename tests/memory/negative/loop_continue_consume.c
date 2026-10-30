// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/loop_forms.c
// Pins: 0038 item 4: a continue path reaches the back edge: the callee consumed the cell before the continue, reported at the body's closing brace
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int via_continue(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
    if (i == 7) {
      dispose(&x);
      continue;
    }
  }
  return i;
}
