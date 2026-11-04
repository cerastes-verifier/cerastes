// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10's follow-up (re-audit S2, M4.5d2v): the back-edge exemption: a block the loop carries (p) beside one it allocates and frees in each iteration (q, a local declared before the loop)
#include <cerastes.h>
#include <stdlib.h>

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    q = (int *)malloc(sizeof(int));
    if (q != NULL) {
      *q = i;
      free(q);
    }
    i += 1;
  }
  free(p);
  return 0;
}
