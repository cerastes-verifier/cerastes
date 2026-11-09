// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning global keeps its allocation across calls; main may end while it holds one.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

static int *g CERASTES_OWNING = NULL;

void install(int v)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = v;
  }
}

int main(void)
{
  install(1);
  install(2);
  return 0;
}
// end::source[]
