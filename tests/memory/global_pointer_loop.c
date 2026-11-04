// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 10 (question 105): owns(*gp) as an automatic loop place (the global pointer's target, through the entry snapshot)
#include <cerastes.h>
#include <stddef.h>

static int h = 0;
static int *gp = &h;

void bump_n(int n)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp) && *gp == 0 && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp) && *gp == n)
{
  int *gp0 = gp;
  (void)(gp0 == gp);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && gp == gp0 && *gp == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *gp += 1;
    i += 1;
  }
}

int main(void)
{
  bump_n(3);
  CERASTES_CHECK(h == 3);
  return 0;
}
