// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_global.c
// Pins: 0054 item 5 (S39 g1x): the loop reallocates the owning global without freeing the block it holds
// Pre-registered from record 0054 item 5 ("Stores"): leak at the assignment
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
    
    g = (int *)malloc(sizeof(int));  // <- here
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
