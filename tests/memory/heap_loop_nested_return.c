// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0047 item 10: nested loops and a return inside them with the block freed first
#include <cerastes.h>
#include <stdlib.h>

int nested(int n)
    CERASTES_REQUIRES(0 <= n && n <= 20)
    CERASTES_ENSURES(r, r == 0 || r == -1 || r == 1)
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
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n)
    CERASTES_DECREASES(n - j)
    while (j < n) {
      *p = i + j;
      j += 1;
    }
    if (i == 3) {
      free(p);
      return 1;
    }
    i += 1;
  }
  free(p);
  return 0;
}
