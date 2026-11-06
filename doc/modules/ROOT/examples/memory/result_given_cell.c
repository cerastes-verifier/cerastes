// expect: verified
// expect: native-ok
// mode: c23, c++20
// A function that holds a global's cell gives it to the caller through its result.
// tag::source[]
#include <cerastes.h>

static int g = 3;

int *take_cell(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(r, cerastes_owns(*r) && r == &g && *r == cerastes_old(g))
{
  return &g;
}

int main(void)
{
  int *p = take_cell();
  int v = *p;
  CERASTES_CHECK(v == 3);
  *p = 5;
  CERASTES_CHECK(g == 5);
  return v - 3;
}
// end::source[]
