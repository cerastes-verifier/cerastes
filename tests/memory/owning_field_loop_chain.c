// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 5, 7 (re-audit S2): a loop frees and replaces a.b->x of a struct local's chain (a.b an owning leaf holding a structure, x its owning field)
// Pre-registered from the final re-audit (probes r4/, intended outcome, not yet run): d_chain_leaf.c: verifies in both modes
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
    free(a.b->x);
    a.b->x = (int *)malloc(sizeof(int));
    if (a.b->x != NULL) {
      *a.b->x = i;
    }
    i = i + 1;
  }
  free(a.b->x);
  free(a.b);
  a.b = NULL;
  return 0;
}
