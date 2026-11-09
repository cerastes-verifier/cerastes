// expect: error lifetime @ L33
// expect: error lifetime @ L34
// expect: error invalid-free @ L38
// (M4.5d2v, question 135: the WF row at L38 is internal again, kept true by the automatic loop invariant)
// expect: error invalid-free @ L39
// expect: error invalid-free @ L39
// (M4.5d2v, question 135: likewise at L39)
// expect: error leak @ L41
// mode: c23, c++20
// twin: tests/memory/heap_loop_two_blocks.c
// Pins: 0047 item 10 (the false alarm it names): a loop that swaps the two pointers that hold two blocks: the places are the blocks the variables designated at entry, and after the loop nothing says which variable points to which, so the frees (and the uses in the body) are reported; the user needs written owns_block atoms in invariants, which M4.5d2b adds; M4.5d2b2a (record 0054 item 4): the same loop with a written owns_block invariant verifies (tests/memory/owns_block_invariant_swap.c and doc example memory/heap_loop_swap_invariant.c); this pin, with no invariant, stays
#include <cerastes.h>
#include <stdlib.h>

int swapper(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    free(p);
    return -1;
  }
  *p = 0;
  *q = 1;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int *t = p;
    p = q;
    q = t;
    i += 1;
  }
  free(p);
  free(q);
  return 0;
}
