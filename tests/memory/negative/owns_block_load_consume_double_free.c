// expect: error invalid-free @ L25
// mode: c23, c++20
// twin: tests/memory/owns_block_load_consume.c
// Pins: 0054 item 2 ("consumed"): the callee took the token with the block, so the caller's free of the dangling pointer is a double free
// Pre-registered from record 0054 item 2 and 0047 item 6: invalid-free at the free call (as heap_double_free.c)
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
  free(p);  // <- here
  return 0;
}
