// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/range_loop_pointer_into_array.c
// Pins: 0060 item 11 (amended): q is reassigned from a pointer parameter that no atom describes (q = r) before the loop (r is live, but no atom holds what it points to), so it may point anywhere: the invariant's q[0] is not framed by the local array's place: permission at the invariant. UNCERTAIN as range_loop_pointer_into_array_past (entry and preserved errors may join)
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>
#include <stddef.h>

int walk_elsewhere(int *r)
    CERASTES_REQUIRES(cerastes_live(r))
{
  int a[4] = {1, 2, 3, 4};
  int *q = a;
  int i = 0;
  q = r;
  if (q == NULL) {
    return -1;
  }
  CERASTES_INVARIANT(0 <= i && i <= 3 && q[0] >= 0) CERASTES_DECREASES(3 - i) while (i < 3) {
    i++;
  }
  return a[0];
}
