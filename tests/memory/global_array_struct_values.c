// expect: verified
// expect: native-ok
// mode: c23
// c-only: the element {.x = 3} leaves its member y out, which g++ -Wextra flags (-Wmissing-field-initializers: struct_init_partial.c's reason), so the test cannot be native-ok in C++; the twins run in both modes
// Pins: 0064 items 3 and 9 (Part C2): a global array of structures with a partial list (a leaf an element's list leaves out is zero, and so are the elements the list does not reach, per leaf) and one with no initializer and internal linkage (zero), read and written by index in main: listed and unreached values checked, a write at a constant index and at a variable index, the other elements and leaves unchanged. Two arrays of two-leaf structures in one function (the plan's rule 9)
// Pre-registered from record 0064 item 9 (Part C2), before any run: verifies
#include <cerastes.h>

struct P {
  int x;
  int y;
};

static struct P g[3] = {{1, 2}, {.x = 3}};
static struct P z[2];

int main(void)
{
  CERASTES_CHECK(g[0].x == 1 && g[0].y == 2);
  CERASTES_CHECK(g[1].x == 3 && g[1].y == 0);
  CERASTES_CHECK(g[2].x == 0 && g[2].y == 0);
  CERASTES_CHECK(z[0].x == 0 && z[1].y == 0);
  g[1].y = 9;
  int i = 2;
  g[i].x = g[1].y + 1;
  CERASTES_CHECK(g[2].x == 10 && g[0].x == 1 && g[1].x == 3);
  return z[1].x + (g[1].y - 9);
}
