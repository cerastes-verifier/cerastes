// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2: a formal's pointee given: ensures(owns(*p)) with no requires atom on *p, the callee holding the cell through the global it equals (requires owns(g) && p == &g); the caller then writes through g
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>

static int g = 0;

void rename_cell(int *p)
    CERASTES_REQUIRES(cerastes_owns(g) && p == &g)
    CERASTES_ENSURES(cerastes_owns(*p) && p == &g)
{
  (void)p;
}

int main(void)
{
  rename_cell(&g);
  g = 4;
  CERASTES_CHECK(g == 4);
  return g - 4;
}
