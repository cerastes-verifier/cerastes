// expect: verified
// expect: native-ok
// mode: c23, c++20
// An owning global (trailing position): a global pointer that owns the block it holds. A function that names owns(g) in its contract receives and returns it, and main may end holding the block (a global with static storage need not release its object).
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

static int *g CERASTES_OWNING = NULL;

void reset(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g);
  g = (int *)malloc(sizeof(int));
  if (g != NULL) {
    *g = 1;
  }
}

int main(void)
{
  reset();
  reset();
  return 0;
}
// end::source[]
