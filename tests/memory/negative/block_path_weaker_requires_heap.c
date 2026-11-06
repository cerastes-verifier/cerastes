// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/block_guard_heap_requires.c
// Pins: 0039 item 7, audit B2 (g_block_path_heap.c): the same with guards that read the heap (*q > 5 in the requires, *q > 0 in the ensures)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B2: permission at the ensures atom (the f__wf partner row)
#include <cerastes.h>
#include <stdlib.h>

void lend(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*q) && (*q > 5 ? cerastes_owns_block(p) : true))
    CERASTES_ENSURES(cerastes_owns(*q) && *q == cerastes_old(*q) && (*q > 0 ? cerastes_owns_block(p) : true));  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  int x = 3;
  lend(p, &x);
  free(p);
  free(p);
  return 0;
}
