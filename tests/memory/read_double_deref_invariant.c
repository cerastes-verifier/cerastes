// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12 (flip of reject_read_double_deref_invariant.c): a read of **pp in an invariant, through the automatic places (the cell p, then x)
#include <cerastes.h>

void walk(int n) CERASTES_REQUIRES(0 <= n)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp >= 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    **pp = i;
    i += 1;
  }
}
