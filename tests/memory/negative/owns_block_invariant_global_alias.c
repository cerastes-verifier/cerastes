// expect: error permission @ L20
// mode: c23, c++20
// twin: tests/memory/owns_block_invariant_global.c
// Pins: 0054 item 4 ("The same pair rows stand between a written site and each owner carrier, an owning global's included"): the written site is on w, a non-owning alias of the owning global's block: both name one block, so the entry pair row fails
// Pre-registered from record 0054 item 4: permission at the invariant clause (entry row; BEST READING of the line)
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING static int *g = NULL;

void churn(int n)
    CERASTES_REQUIRES(cerastes_owns(g) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(g))
{
  int *w = g;
  if (w != NULL) {
    *w = 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && (w == NULL || cerastes_owns_block(w)))  // <- here
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
}

int main(void)
{
  churn(3);
  return 0;
}
