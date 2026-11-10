// expect: error leak @ L35
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_chain.c
// Pins: 0055 item 7: the loop replaces a.b->x without freeing the block it holds: the back-edge row at the loop body's closing brace (d_chain_leaf_drop.c)
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): record 0055 items 4, 7: leak at the loop body's closing brace (a best reading; to be confirmed by the run, question 160)
#include <cerastes.h>
#include <stdlib.h>

struct leaf {
  int *x CERASTES_OWNING;
  int k;
};

struct mid {
  struct leaf *b CERASTES_OWNING;
  int m;
};

int f(void)
{
  struct mid a = { NULL, 0 };
  a.b = (struct leaf *)calloc(1, sizeof(struct leaf));
  if (a.b == NULL) {
    return 0;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    a.b->x = (int *)malloc(sizeof(int));
    if (a.b->x != NULL) {
      *a.b->x = i;
    }
    i = i + 1;
  }  // <- here
  free(a.b->x);
  free(a.b);
  a.b = NULL;
  return 0;
}
