// expect: error invalid-free @ L27
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_loop_param_cursor.c
// Pins: 0050 item 5 (audit round 2 S3): the vlibc block the cursor parameter was pointed at is freed twice
// Pre-registered from record 0050 item 5 and heap_loop_param_cursor_double_free.c: invalid-free at the second vlibc_free
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(int *t, int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
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
    t = p;
    *t = i;
    i += 1;
  }
  vlibc_free(p);
  vlibc_free(p);  // <- here
  return 0;
}
