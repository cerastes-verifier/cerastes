// expect: error permission @ L12
// mode: c23, c++20
// The function promises owns(*r) and holds nothing of g: it cannot give what it does not have.
// tag::source[]
#include <cerastes.h>

static int g = 3;

int *take_cell(void)
    CERASTES_ENSURES(r, cerastes_owns(*r) && r == &g)
{
  return &g;
}
// end::source[]
