// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0063 item 9 (audit round 2, S1, p03b): an element of a pointer array holding another array's decay, both of internal linkage: g stays known and tab[0] is the address of its first element
// Pre-registered from audit round 2 of record 0063 (S1), before any run: verifies
#include <cerastes.h>

static int g[2] = {1, 2};
static int *tab[1] = {g};

int main()
{
  CERASTES_CHECK(g[0] == 1);
  CERASTES_CHECK(tab[0] == &g[0]);
  return tab[0][1] - 2;
}
