// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: a parameter place (owns(*r)) and a heap place in one loop
#include <cerastes.h>
#include <stdlib.h>

int mix(int *r, int n)
    CERASTES_REQUIRES(cerastes_owns(*r))
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(cerastes_owns(*r))
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    *r = *p + 1;
    i += 1;
  }
  free(p);
  return 0;
}
