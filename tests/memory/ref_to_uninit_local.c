// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 4 (target v, amends 0042 item 2), default mode: the marker on a local pointer documents intent and changes no verdict: a marked local receiving malloc is written before it is read, and a marked local receiving calloc (initialized) is read at once
// Pre-registered from record 0052 item 4: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int fill(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 3;
  int v = *p;
  free(p);
  return v;
}

int zeroed(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_REF_TO_UNINIT int *q = (int *)calloc(1, sizeof(int));
  if (q == NULL) {
    return -1;
  }
  int v = *q;
  free(q);
  return v;
}
