// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10's follow-up (re-audit B1, M4.5d2v): a pointer parameter the loop assigns (t = p) beside a block it does not own: the parameter's copy and the allocated block are both places of the loop
#include <cerastes.h>
#include <stdlib.h>

int f(int *t, int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
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
    t = p;
    *t = i;
    i += 1;
  }
  free(p);
  return 0;
}
