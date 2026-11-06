// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 6e: free(p) and a use of p in different statements: nothing is unsequenced
// Pre-registered from record 0052 item 6e: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int two(int a, int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == a);

int use_two(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  int v = two(0, p);
  free(p);
  return v;
}
