// expect: error leak @ L29
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_owning_loop_replace.c
// Pins: 0052 item 3: the vlibc loop allocates into the owning local q without freeing the old block: leak at the assignment
// Pre-registered from record 0052 item 3: leak at the assignment
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
    q = (int *)vlibc_malloc(sizeof(int));  // <- here
    i += 1;
  }
  free(p);
  vlibc_free(q);
  return 0;
}
