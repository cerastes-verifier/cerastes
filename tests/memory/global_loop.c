// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 item 10: in main every global is an automatic place of the loop: g, which nothing in the loop can write, keeps its value; h is written by name, its value is the user's invariant
#include <cerastes.h>

static int g = 3;
static int h = 7;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && h == 7 + i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    h += 1;
    i += 1;
  }
  CERASTES_CHECK(g == 3 && h == 17);
  return g - g;
}
