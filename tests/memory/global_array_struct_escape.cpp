// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0064 item 9 (0041 item 3, per leaf; 0063 item 9's escape rule): with internal linkage the array of structures whose pointer leaf holds the address of h keeps h and every element known in main (the escape twin gives the array external linkage: negative/global_array_struct_escape_tab.cpp)
// Pre-registered from record 0064 item 9 (Part C2), before any run: verifies
#include <cerastes.h>

struct Q {
  int *p;
  int y;
};

static int h = 7;
static struct Q tab[2] = {{&h, 1}, {0, 2}};

int main()
{
  CERASTES_CHECK(h == 7);
  CERASTES_CHECK(tab[0].p == &h && tab[1].p == 0);
  CERASTES_CHECK(tab[0].y == 1 && tab[1].y == 2);
  return tab[1].y - 2;
}
