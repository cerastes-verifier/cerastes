// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owning_loop_free_on_last.c
// Pins: 0052 item 3 (audit B2, a dangling owner at a back edge; revised record 0052 item 3 Loops): the loop frees the owner's block on its last iteration and leaves v pointing at it: at the back edge v != null but held_v is false (the free cleared it), so the loop's back-edge ending row v != null ==> held_v fails: permission at the back edge, which is the designed error (the owner must be set to NULL after a free inside a loop: the verified twin tests/memory/owning_loop_free_on_last.c). Not a false alarm any more
// Pre-registered from record 0052 item 3 (audit round 1, S12): permission at the loop body's closing brace (the back-edge row, as the run gives; the free after the loop makes the program double free, so the state the invariant gives at the exit holds the block and no leak follows)
#include <cerastes.h>
#include <stdlib.h>

int last_frees(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 1;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n)
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    if (k == 0) {
      free(v);
    }
  }  // <- here
  free(v);
  return 0;
}
