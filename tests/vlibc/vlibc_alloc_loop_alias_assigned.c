// expect: verified
// expect: native-ok
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// Pins: 0050 item 5 (audit round 2 S3): vlibc version of heap_loop_alias_assigned.c: a variable the loop assigns (q = p) beside a vlibc block held through p
// Pre-registered from record 0050 item 5: verifies
#include <vlibc/stdlib.h>
#include <cerastes.h>

int reassigned(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = p;
  *p = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    q = p;
    *q = i;
    i += 1;
  }
  vlibc_free(p);
  return 0;
}
