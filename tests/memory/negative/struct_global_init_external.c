// expect: error check @ L19
// c-only: C++ external linkage gives g2 an unknown start in main (decision 0044 item 8): the test is about the C initial values
// twin: tests/memory/struct_global_init.c
// Pins: 0044 item 8: g2.a starts at 2 (C, external linkage with an initializer): g2.a == 3 fails
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
  CERASTES_CHECK(g.a == 1 && g.p == &h && h == 7);
  CERASTES_CHECK(g2.a == 3 && g2.p == &h);  // <- here
  g.a = 9;
  CERASTES_CHECK(g.a == 9);
  return (g.p == &h ? 1 : 0) + g2.a + g.a;
}
