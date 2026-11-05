// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 items 3, 5 (0047 item 2): vlibc_malloc and vlibc_free through the explicit header, and the platform's malloc and free beside them in one function: each block is freed by its own allocator's routine
// Pre-registered from record 0050 items 3, 5: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  int v = *p;
  vlibc_free(p);
  return v;
}

int both(void)
    CERASTES_ENSURES(r, r == 3 || r == -1)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    vlibc_free(p);
    return -1;
  }
  *p = 1;
  *q = 2;
  int v = *p + *q;
  free(q);
  vlibc_free(p);
  return v;
}
