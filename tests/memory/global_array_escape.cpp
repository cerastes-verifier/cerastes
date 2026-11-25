// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0063 item 9 (0041 item 3, per element): with internal linkage the pointer array and the pointer global keep h and g known in main (the escape twins give them external linkage: negative/global_array_escape_tab.cpp, global_array_escape_decay.cpp)
// Pre-registered from record 0063 item 9 (Part B), before any run: verifies
#include <cerastes.h>

static int h = 7;
static int *tab[2] = {&h, 0};
static int g[3] = {1, 2, 3};
static int *gp = g;

int main()
{
  CERASTES_CHECK(h == 7);
  CERASTES_CHECK(g[1] == 2);
  (void)tab[1];
  return gp == 0 ? 1 : 0;
}
