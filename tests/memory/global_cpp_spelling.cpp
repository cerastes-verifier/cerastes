// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0041 item 7: the C++ spellings cerastes::owns(g), cerastes::reads(g) and cerastes::old(g) name a global like the C ones
#include <cerastes.h>

static int g = 3;

void bump()
    CERASTES_REQUIRES(cerastes::owns(g) && g < 100)
    CERASTES_ENSURES(cerastes::owns(g) && g == cerastes::old(g) + 1)
{
  g += 1;
}

int peek()
    CERASTES_REQUIRES(cerastes::reads(g))
    CERASTES_ENSURES(r, cerastes::reads(g) && r == g)
{
  return g;
}

int main()
{
  bump();
  int v = peek();
  CERASTES_CHECK(v == 4 && g == 4);
  (void)v;
  return 0;
}
