// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: a heap pointer and a pointer to a local cell in one loop: the guards keep the places apart
#include <cerastes.h>
#include <stdlib.h>

int alias(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = &x;
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    *q = i;
    i += 1;
  }
  free(p);
  return x;
}
