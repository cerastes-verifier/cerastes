// expect: error check @ L16
// mode: c23, c++20
// twin: tests/memory/not_invalidating_no_atom.c
// Pins: 0058 item 2 ("No atom rooted at the parameter"): the same bodiless contract, and the caller checks a result the ensures does not give (r == n only): the marker adds no promise through p, so the check may not hold
// Pre-registered from record 0058 item 2: one check error at the check
#include <cerastes.h>

int pass(int *p CERASTES_NOT_INVALIDATING, int n)
    CERASTES_ENSURES(r, r == n);

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  int x = 1;
  int v = pass(&x, 3);
  CERASTES_CHECK(v == 4);  // <- here
  x = 2;
  return v;
}
