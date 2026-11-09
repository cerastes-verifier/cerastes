// expect: error permission @ L24
// mode: c23, c++20
// twin: tests/memory/owns_block_global_loop.c
// Pins: 0054 item 2 ("Loops") and audit round 2 item 2 (probe l9): the loop frees the global's block on one iteration and writes *g on the next
// Pre-registered from the audit probe l9_loop_global_free_in_loop as observed: permission at the loop body's closing brace
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
    if (i == 1) {
      free(g);
    }
    i += 1;
  }  // <- here
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
