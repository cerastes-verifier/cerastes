// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 3 (0047 item 7): a vlibc block is freed on every path
// Pre-registered from record 0050 item 3: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int branches(int c)
    CERASTES_ENSURES(r, r == 0 || r == 1 || r == -1)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  if (c != 0) {
    *p = 1;
    vlibc_free(p);
    return 1;
  }
  vlibc_free(p);
  return 0;
}
