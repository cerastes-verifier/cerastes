// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 items 7 and 9: a reads(g) callee leaves g as it was, a callee whose contract names no global cannot change g or h (the caller keeps both values)
#include <cerastes.h>

static int g = 3;
static int h = 7;

int peek(void)
    CERASTES_REQUIRES(cerastes_reads(g))
    CERASTES_ENSURES(r, cerastes_reads(g) && r == g)
{
  return g;
}

void other(void)
{
}

int main(void)
{
  int a = peek();
  other();
  CERASTES_CHECK(a == 3 && g == 3 && h == 7);
  (void)a;
  return h - h;
}
