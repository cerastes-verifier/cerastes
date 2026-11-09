// expect: error permission @ L15
// mode: c23, c++20
// A function that frees the owning global's block must set the global to NULL before it returns.
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
