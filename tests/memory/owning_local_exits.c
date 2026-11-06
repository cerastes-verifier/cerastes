// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 3: the lifetime-end row at break, continue and a goto out of the owning local's block, and at a return inside the loop: each frees first
// Pre-registered from record 0052 item 3: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

int scan(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      return -1;
    }
    *p = i;
    if (i == 5) {
      free(p);
      break;
    }
    if (i == 3) {
      free(p);
      i += 1;
      continue;
    }
    free(p);
    i += 1;
  }
  return 0;
}

int early(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  {
    CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      return -1;
    }
    *p = c;
    if (c == 0) {
      free(p);
      goto done;
    }
    free(p);
  }
done:
  return 0;
}
