// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 15 (Part D's loop column) and 0047 item 10 (a loop in a function with owns_block in both lists): the callee holds the block of its parameter, a structure with an array member, and fills the array in a loop; the caller allocates it, passes it and frees it
// Pre-registered from the plan's Part D table (section 7: `loop_places` and `loop` with an array leaf's lines) and record 0064 item 15 (heap storage), read with record 0047 item 10 (a heap block is a place of a loop, with a back-edge row), and il-analyst's probe d1probe/b2 (case e1e2d_loop_heap_param_block), before any run of this file: verifies in both modes; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

void fill(struct S *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(cerastes_owns_block(p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = i;
    i = i + 1;
  }
  p->n = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  fill(p);
  free(p);
  return 0;
}
