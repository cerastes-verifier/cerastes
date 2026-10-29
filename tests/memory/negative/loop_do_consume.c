// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/loop_forms.c
// Pins: 0038 item 4: the back edge of a do loop is after its exit test; a callee that consumes the cell on the way round again is reported at the body's closing brace (the exit path keeps the cell, so that is the only failure)
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int via_do(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  int x = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i < n)
  CERASTES_DECREASES(n - i)
  do {
    i += 1;
    if (i < n) {
      dispose(&x);
    }
  } while (i < n);
  return i;
}
