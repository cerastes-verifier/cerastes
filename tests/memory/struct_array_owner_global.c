// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (293; 0054 item 5: owning globals) for a global `struct S *` whose structure has an array member of scalars: init replaces the block (calloc: every element zero), a user reads an element of the global's block, destroy frees it, and main's held_g is false afterwards. Beyond the plan's list of Part D files (question 351, item 2 (c)): the owning-global pair
// Pre-registered from record 0064 item 15 and the precedent owning_global_consume.c, before any run: verifies in both modes (unmeasured for an array leaf)
#include <cerastes.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

CERASTES_OWNING static struct S *g = NULL;

void init(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (struct S *)calloc(1, sizeof(struct S));
  if (g != NULL) {
    g->a[1] = 5;
  }
}

void destroy(void)
    CERASTES_REQUIRES(cerastes_owns(g))
{
  free(g);
}

int main(void)
{
  init();
  destroy();
  return 0;
}
