// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5 (0047 item 10): a vlibc block held across a loop that writes it keeps its allocator, so vlibc_free after the loop is accepted
// Pre-registered from record 0050 item 5: verifies (the loop carries the token's value)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int loop_holds_block(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    *p = i;
    i += 1;
  }
  vlibc_free(p);
  return 0;
}
