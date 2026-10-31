// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 10 (0038's havoc rule): a function that owns *p and g loops with *p = i: a store through a pointer of g's field type can reach g, so g's value is the user's invariant
#include <cerastes.h>

static int g = 5;

void fill(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(g) && g == 5)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_owns(g) && g == 5)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && g == 5)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    *p = i;
    i += 1;
  }
}

int main(void)
{
  int x = 0;
  fill(&x);
  CERASTES_CHECK(g == 5);
  return g - g;
}
