// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 2 (retires reject_owns_block_load.c's shape): a requires site owns_block(*pp) with no ensures partner CONSUMES the block (the callee frees it); the partnered owns(*pp) comes back, so the caller's cell keeps a dangling value that the caller overwrites
// Pre-registered from record 0054 item 2 ("consumed", S39 a'): verifies in both modes (s39_a_consume_borrow.vpr)
#include <cerastes.h>
#include <stdlib.h>

void dispose(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp))
{
  free(*pp);
}

int make_and_dispose(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 5;
  dispose(&p);
  p = NULL;
  return 0;
}
