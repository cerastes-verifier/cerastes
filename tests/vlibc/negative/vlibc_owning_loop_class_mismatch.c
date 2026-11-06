// expect: error permission @ L21
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_owning_loop_null_entry.c
// Pins: 0052 item 3 Loops, shape L8: the owner enters the loop holding a block of the platform's allocator while the loop assigns it only from vlibc_malloc, so the class of the loop is vlibc: the entry class row fails (the owner holds a block of another allocator than the one this loop's assignments give it): permission at the loop
// Pre-registered from record 0052 item 3 Loops (il-analyst's probes build/m4.5d2b/loop/p): permission at the loop (the entry class row; the line is the first run's)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int fill(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    return -1;
  }
  *q = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {  // <- here
    vlibc_free(q);
    q = (int *)vlibc_malloc(sizeof(int));
    i += 1;
  }
  vlibc_free(q);
  return 0;
}
