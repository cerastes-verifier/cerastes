// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5 (audit round 2 S3): vlibc version of heap_loop_reassigned_free.c: a pointer local the loop assigns beside a vlibc block stored in a member, freed through the member after the loop
// Pre-registered from record 0050 item 5: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

struct L {
  int *q;
};

int f(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  struct L *l = (struct L *)vlibc_malloc(sizeof(struct L));
  if (l == NULL) {
    return -1;
  }
  int *a = (int *)vlibc_malloc(sizeof(int));
  if (a == NULL) {
    vlibc_free(l);
    return -1;
  }
  l->q = a;
  int *c = a;
  *c = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    c = a;
    *c = i;
    i += 1;
  }
  vlibc_free(l->q);
  vlibc_free(l);
  return 0;
}
