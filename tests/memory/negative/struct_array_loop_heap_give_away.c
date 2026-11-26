// expect: error permission @ L28
// mode: c23, c++20
// twin: tests/memory/struct_array_loop_heap_fill.c
// Pins: 0064 item 15 (Part D's loop column) and 0047 item 10 (the back-edge row of a heap place): the body frees the block the loop holds, so the back edge no longer has the array part's permissions: permission at the body's closing brace (the precedent negative/heap_loop_free_in_body.c; the array part's marked rows stand before the cells' rows, and the self-test case e1e2d_loop_heap_give_away_fail reports exactly one error at them)
// Pre-registered from the plan's Part D table (section 7: `loop_places` and `loop` with an array leaf's lines) and record 0064 item 15 (heap storage), read with record 0047 item 10 (a heap block is a place of a loop, with a back-edge row), the precedent heap_loop_free_in_body.c (permission at the loop body's closing brace) and the self-test case e1e2d_loop_heap_give_away_fail (one `insufficient.permission`), before any run; the rule is record 0064 item 15's clause "A heap block's array part in a loop" (written 2026-11-26 with il-analyst's code), and these expectations are predictions from it, not measurements
#include <cerastes.h>
#include <stddef.h>
#include <stdlib.h>

struct S {
  int a[4];
  int n;
};

int give_away(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  struct S *p = (struct S *)calloc(1, sizeof(struct S));
  if (p == NULL) {
    return -1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    free(p);
    i = i + 1;
  }  // <- here
  free(p);
  return 0;
}
