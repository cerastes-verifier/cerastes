// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: a variable the loop assigns (q = p): its place is the entry value (a ghost), which is p's block, held once
#include <cerastes.h>
#include <stdlib.h>

int reassigned(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = p;
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    *q = i;
    i += 1;
  }
  free(p);
  return 0;
}
