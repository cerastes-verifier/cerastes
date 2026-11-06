// expect: error permission @ L11
// mode: c23, c++20
// twin: tests/memory/block_guard_heap_requires.c
// Pins: 0039 item 7 (question 72), audit B2 (g_block_path.c): a bodiless borrow whose ensures guard (c > 0) is weaker than its requires guard (c > 5): where 0 < c <= 5 the ensures returns a block the call did not take: the partner-by-path row of f__wf fails at the ensures atom (the double free after lend(p, 3) must never verify)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) B2: permission at the ensures atom (the f__wf partner row)
#include <cerastes.h>
#include <stdlib.h>

void lend(int *p, int c)
    CERASTES_REQUIRES(c > 5 ? cerastes_owns_block(p) : true)
    CERASTES_ENSURES(c > 0 ? cerastes_owns_block(p) : true);  // <- here

int main(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return 1;
  }
  *p = 1;
  lend(p, 3);
  free(p);
  free(p);
  return 0;
}
