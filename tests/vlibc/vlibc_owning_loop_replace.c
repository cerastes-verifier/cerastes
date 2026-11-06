// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0052 item 3 (0047 item 10 as amended at M4.5d2v): the verified twin of vlibc_alloc_loop_replace_false_alarm.c: q is an owning local holding a vlibc block, the loop frees it with vlibc_free and allocates another; the lifetime and reassignment rows read the token, whichever allocator made the block
// Pre-registered from record 0052 item 3: verifies; the record does not say in so many words that an owning local may hold a vlibc block (the plan's Inputs say the vlibc false alarm is lifted by owning variables)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int mixed(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  CERASTES_OWNING int *q = (int *)vlibc_malloc(sizeof(int));
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
    vlibc_free(q);
    q = (int *)vlibc_malloc(sizeof(int));
    i += 1;
  }
  free(p);
  vlibc_free(q);
  return 0;
}
