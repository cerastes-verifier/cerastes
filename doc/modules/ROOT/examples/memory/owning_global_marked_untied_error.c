// expect: error uninitialized @ L25
// mode: c23, c++20
// Without the equality the callee may have replaced the block, so the caller cannot read what it wrote.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

int *g CERASTES_OWNING CERASTES_REF_TO_UNINIT;

void touch(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
}

void use(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  if (g != NULL) {
    *g = 1;
  }
  touch();
  if (g != NULL) {
    int y = *g;
    *g = y;
  }
}
// end::source[]
