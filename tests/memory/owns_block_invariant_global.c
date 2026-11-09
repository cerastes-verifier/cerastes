// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 4 and item 5 ("Loops"): a written owns_block(g) on an owning global in a loop invariant (g == NULL || owns_block(g)): it merges with the global's own carrier; the loop frees and reallocates g every iteration, and main ends holding g's block (exempt)
// Pre-registered from record 0054 items 4 and 5 (S39 g1, s39_g_owning_global_loop.vpr): verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void churn(int n)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (g == NULL || cerastes_owns_block(g)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    free(g);
    g = (int *)malloc(sizeof(int));
    if (g != NULL) {
      *g = i;
    }
    i += 1;
  }
}

int main(void)
{
  churn(3);
  return 0;
}
