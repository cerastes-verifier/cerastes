// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12: a written atom on owns(*gp) of a pointer global in an invariant (the root is the global's cell)
#include <cerastes.h>
#include <stddef.h>

static int h = 0;
static int *gp = &h;

void walk(int n)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp))
{
  int *gp0 = gp;
  (void)(gp0 == gp);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && gp == gp0 && cerastes_owns(*gp))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *gp = i;
    i += 1;
  }
}

int main(void)
{
  walk(2);
  return 0;
}
