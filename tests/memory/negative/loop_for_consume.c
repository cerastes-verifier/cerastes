// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/loop_forms.c
// Pins: 0038 item 4: the back edge of a for loop is after the step; a consuming callee in the body is reported at the body's closing brace
#include <cerastes.h>

void dispose(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p));

int via_for(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int i;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  for (i = 0; i < n; i += 1) {
    dispose(&x);
  }
  return i;
}
