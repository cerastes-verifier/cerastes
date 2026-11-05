// expect: error invalid-free @ L25
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_loop_after.c
// Pins: 0050 item 5: the loop carries the value naming vlibc's allocator, so the platform's free after the loop is invalid-free
// Pre-registered from record 0050 item 5: invalid-free at the free after the loop
#include <vlibc/stdlib.h>
#include <cerastes.h>

int loop_holds_block(int n)
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
    *p = i;
    i += 1;
  }
  free(p);  // <- here
  return 0;
}
