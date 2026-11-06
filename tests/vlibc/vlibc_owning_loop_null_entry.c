// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0052 item 3 Loops, shape L8: a vlibc heap program, an owner that is NULL at the loop's entry and assigned in the loop only from vlibc_malloc: the loop's allocator class is vlibc (no entry token ghost needed), so the class rows hold
// Pre-registered from record 0052 item 3 Loops (il-analyst's probes build/m4.5d2b/loop/p): verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *q = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    vlibc_free(q);
    q = (int *)vlibc_malloc(sizeof(int));
    i += 1;
  }
  vlibc_free(q);
  return 0;
}
