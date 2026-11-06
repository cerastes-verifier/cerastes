// expect: error permission @ L25
// mode: c23, c++20
// twin: tests/memory/owning_loop_free_on_last.c
// Pins: 0052 item 3 (audit B2): the same loop and invariant without the v = NULL: after the free on the last iteration v is not NULL, so the owner's back-edge row v != null ==> held_v fails before the invariant is exhaled: permission at the closing brace of the loop body (the first run's row; set from invariant-preserved under question 160 and record 0052's revision of item 3 Loops)
// Pre-registered from record 0052 item 3 (audit round 1, S12): permission at the back-edge row
#include <cerastes.h>
#include <stdlib.h>

int last_frees(int n)
    CERASTES_REQUIRES(1 <= n && n <= 100)
{
  CERASTES_OWNING int *v = (int *)malloc(sizeof(int));
  if (v == NULL) {
    return -1;
  }
  *v = 1;
  int k = n;
  CERASTES_INVARIANT(0 <= k && k <= n && (k > 0 || v == NULL))
  CERASTES_DECREASES(k)
  while (k > 0) {
    k -= 1;
    if (k == 0) {
      free(v);
    }
  }  // <- here
  return 0;
}
