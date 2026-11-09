// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 ("Loops": "on a global") and audit round 2 item 2 (probe l9): a requires site owns_block(g) on a pointer global is an automatic loop place: the loop writes *g, and f frees the block and clears g after it
// Pre-registered from the audit probe l9 without its free in the loop: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

static int *g = NULL;

void f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 10 && cerastes_owns(g) && cerastes_owns_block(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *g = i;
    i += 1;
  }
  free(g);
  g = NULL;
}

int main(void)
{
  g = (int *)malloc(sizeof(int));
  if (g == NULL) {
    return 1;
  }
  *g = 0;
  f(3);
  return 0;
}
