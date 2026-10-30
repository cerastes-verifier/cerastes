// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 11: g++ + *p with p != &g known from the two owns atoms (the path-precise check passes); g + bump_h() where bump_h owns the other global h: a memory call and an access that name different places
#include <cerastes.h>

static int g = 3;
static int h = 7;

int bump_h(void)
    CERASTES_REQUIRES(cerastes_owns(h) && h < 100)
    CERASTES_ENSURES(r, cerastes_owns(h) && h == cerastes_old(h) + 1 && r == h)
{
  h += 1;
  return h;
}

int mix(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g) && cerastes_owns(h) && 0 <= g && g < 100 && 0 <= h && h < 100 && 0 <= *p && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && cerastes_owns(g) && cerastes_owns(h) && g == cerastes_old(g) + 1 && h == cerastes_old(h) + 1)
{
  int a = g++ + *p;
  int b = g + bump_h();
  return a + b;
}

int main(void)
{
  int x = 5;
  mix(&x);
  CERASTES_CHECK(g == 4 && h == 8);
  return 0;
}
