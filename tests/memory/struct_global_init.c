// expect: verified
// expect: native-ok
// c-only: C++ external linkage gives g2 an unknown start in main (decision 0044 item 8): the test is about the C initial values
// Pins: 0044 item 8 (question 84): a struct global with an initializer {1, &h} holds its values in main, per member, for internal and (in C) external linkage; the pointer member is the address of the global h
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
  CERASTES_CHECK(g2.a == 2 && g2.p == &h);
  g.a = 9;
  CERASTES_CHECK(g.a == 9);
  return (g.p == &h ? 1 : 0) + g2.a + g.a;
}
