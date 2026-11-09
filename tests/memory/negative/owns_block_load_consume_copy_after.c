// expect: error lifetime @ L25
// mode: c23, c++20
// twin: tests/memory/owns_block_load_consume.c
// Pins: 0054 item 2 ("consumed"): the callee took the block, the caller's cell keeps a dangling value, and using the dangling pointer (here a void cast, as lifetime_void_cast_after_block.c) is a use after the object ended (0032 item 6; the plan calls the uses of it lifetime)
// Pre-registered from record 0054 item 2 and the plan's Part A ("its uses are lifetime"): lifetime at the use (BEST READING: a dereference of the dangling pointer may instead be permission, see the report)
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
  (void)p;  // <- here
  p = NULL;
  return 0;
}
