// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5 (audit round 2 B1): a vlibc block reached through a bodiless function's result (ensures r == a or r == b) is freed with vlibc_free, the other freed under the condition: the token rows' reads are framed
// Pre-registered from audit round 2 B1 (a2/g1_vlibc_pick.c; the platform twin verifies): verifies (written before il-analyst's fix: until it is built it exits 3)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int *pick(int *a, int *b, int c)
    CERASTES_ENSURES(r, (c != 0 && r == a) || (c == 0 && r == b));

int f(int c)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)vlibc_malloc(sizeof(int));
  if (q == NULL) {
    vlibc_free(p);
    return -1;
  }
  int *r = pick(p, q, c);
  vlibc_free(r);
  if (c != 0) {
    vlibc_free(q);
  } else {
    vlibc_free(p);
  }
  return 0;
}
