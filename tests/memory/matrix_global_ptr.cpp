// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0064 items 9 and 3 (Part E; the cell type of a global matrix; as global_array_escape.cpp): a static matrix of pointers whose first cell holds the address of h keeps h and every cell known in main (internal linkage: no other unit can write them)
// Pre-registered from record 0064 items 3 and 9 (Part E), before any run: verifies
#include <cerastes.h>

static int h = 7;
static int *tab[2][2] = {{&h, 0}, {0, 0}};

int main()
{
  CERASTES_CHECK(h == 7);
  CERASTES_CHECK(tab[0][1] == 0);
  (void)tab[1][0];
  return 0;
}
