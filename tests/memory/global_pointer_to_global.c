// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 items 6 and 7: &g stored in a local pointer and written through while owning g; *&g is g
#include <cerastes.h>

static int g = 3;

void set_via_pointer(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g) && g == 5)
{
  int *p = &g;
  *p = 5;
  CERASTES_CHECK(*p == 5);
}

int via_star(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g)
{
  return *&g;
}

int main(void)
{
  set_via_pointer();
  CERASTES_CHECK(g == 5);
  int v = via_star();
  CERASTES_CHECK(v == 5);
  (void)v;
  return 0;
}
