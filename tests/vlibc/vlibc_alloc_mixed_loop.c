// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5: a loop that holds a platform block and a vlibc block, each with its own allocator, and the frees after it
// Pre-registered from record 0050 item 5: verifies (each token place carries its own value)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int mixed(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)vlibc_malloc(sizeof(int));
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
    *q = *p;
    i += 1;
  }
  free(p);
  vlibc_free(q);
  return 0;
}
