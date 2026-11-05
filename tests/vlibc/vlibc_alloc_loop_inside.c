// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5 (0047 item 7): a vlibc block allocated and freed in the body of a loop: nothing is held at the back edge
// Pre-registered from record 0050 item 5: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int loop_allocates(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *p = (int *)vlibc_malloc(sizeof(int));
    if (p != NULL) {
      *p = i;
      vlibc_free(p);
    }
    i += 1;
  }
  return 0;
}
