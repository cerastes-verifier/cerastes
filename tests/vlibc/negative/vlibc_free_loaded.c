// expect: error invalid-free @ L23
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_pick.c
// Pins: 0050 item 5 (audit round 2 B1): vlibc_free of a pointer the function only borrows through owns(*pp) holds no token for the block it points to: invalid-free at the free (it exited 3 before the token rows' reads were framed)
// Pre-registered from audit round 2 B1 (a2/e2_vlibc_free_loaded.c; the platform analog gives one invalid-free): invalid-free at vlibc_free(*pp), one error (written before il-analyst's fix: until it is built it exits 3)
#include <vlibc/stdlib.h>
#include <cerastes.h>

void g(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  int *q1 = (int *)vlibc_malloc(sizeof(int));
  if (q1 == NULL) {
    return;
  }
  int *q2 = (int *)vlibc_malloc(sizeof(int));
  if (q2 == NULL) {
    vlibc_free(q1);
    return;
  }
  vlibc_free(*pp);  // <- here
  vlibc_free(q1);
  vlibc_free(q2);
}
