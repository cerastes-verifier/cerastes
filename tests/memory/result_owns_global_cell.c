// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (given): the callee keeps its requires owns(g) (0037 item 13) and hands the cell back through *r: owns(*r) has no requires partner, and the callee holds g beyond its partnered returns, so the atom gives the caller an output amount; the caller writes *r and sees g
// Pre-registered from record 0052 item 2: verifies in both modes
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
