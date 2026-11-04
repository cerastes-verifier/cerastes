// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 11, 12 (question 109): a heap-reading guard in a loop invariant: *pp == NULL || owns(**pp) as a written atom, its guard read through the automatic place *pp
#include <cerastes.h>
#include <stddef.h>

void fill(int **pp, int n)
    CERASTES_REQUIRES(cerastes_owns(*pp) && (*pp == NULL || cerastes_owns(**pp)) && 0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*pp == NULL || cerastes_owns(**pp)))
{
  int *p0 = *pp;
  (void)(p0 == *pp);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && *pp == p0 && (*pp == NULL || cerastes_owns(**pp)))
  CERASTES_DECREASES(n - i)
  while (i < n) {
    if (*pp != NULL) {
      **pp = i;
    }
    i += 1;
  }
}
