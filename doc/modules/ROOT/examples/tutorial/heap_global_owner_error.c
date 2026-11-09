// expect: error permission @ L16
// mode: c23, c++20
// A function that frees the owning global's block must set the global to NULL before it returns.
// Derived from heap_global_owner.c.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

static int *g CERASTES_OWNING = NULL;

void discard(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
}

int main(void)
{
  discard();
  return 0;
}
// end::source[]
