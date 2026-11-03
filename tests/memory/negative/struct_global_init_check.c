// expect: error check @ L18
// c-only: C++ external linkage gives g2 an unknown start in main (decision 0044 item 8): the test is about the C initial values
// twin: tests/memory/struct_global_init.c
// Pins: 0044 item 8: g.a starts at 1, not 2: check at the start of main
#include <cerastes.h>

struct P {
  int a;
  int *p;
};

static int h = 7;
static struct P g = {1, &h};
struct P g2 = {2, &h};

int main(void)
{
  CERASTES_CHECK(g.a == 2 && g.p == &h && h == 7);  // <- here
  CERASTES_CHECK(g2.a == 2 && g2.p == &h);
  g.a = 9;
  CERASTES_CHECK(g.a == 9);
  return (g.p == &h ? 1 : 0) + g2.a + g.a;
}
