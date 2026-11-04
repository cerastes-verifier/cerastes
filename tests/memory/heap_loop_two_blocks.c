// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: two blocks held through two locals across a loop: both stay held (the tokens' guards keep the two blocks apart), freed after it
#include <cerastes.h>
#include <stdlib.h>

int two(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    *q = *p + 1;
    i += 1;
  }
  free(q);
  free(p);
  return 0;
}
