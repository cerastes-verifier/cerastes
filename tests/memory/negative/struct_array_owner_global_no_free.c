// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/struct_array_owner_global.c
// Pins: 0064 item 15 (293; 0054 item 5 "Every other function returns the block through its ensures atom on g, or frees it"), the owning-global twin of question 351 item 2 (c): destroy takes the cell with its block, which has an array member, and neither frees nor returns it: leak at the closing brace of destroy (the precedent negative/owning_global_consume_no_free.c)
// Pre-registered from the precedent owning_global_consume_no_free.c: leak at the closing brace of destroy (unmeasured for an array leaf)
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
}  // <- here

int main(void)
{
  init();
  destroy();
  return 0;
}
