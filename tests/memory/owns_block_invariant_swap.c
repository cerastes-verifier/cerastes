// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 4 (retires reject_owns_block_invariant.c and cures heap_loop_swap_false_alarm.c's program, which stays pinned without an invariant): the loop that swaps the two pointers holding two blocks, with a written owns_block site per pointer in the invariant: the written carriers are keyed on the current values, the pair rows block_of(p) != block_of(q) hold at entry and at the back edge, and the frees after the loop find the blocks
// Pre-registered from record 0054 item 4 and S39 c1 (s39_c_swap_written_invariant.vpr): verifies in both modes
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
  CERASTES_INVARIANT(0 <= i && i <= n && (p == NULL || cerastes_owns_block(p)) && (q == NULL || cerastes_owns_block(q)))
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
