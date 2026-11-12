// expect: error permission @ L14
// mode: c23, c++20
// twin: tests/memory/range_loop_pointer_into_array.c
// Pins: 0060 item 11 (amended): the invariant reads q[4], one past the four-element array the pointer points into: no place of the loop holds it, so the read is unframed: registered as one permission at the invariant (this loop invariant reads q[e], which this loop does not hold). UNCERTAIN: the clause has no value the loop can keep, so an invariant-entry and an invariant-preserved error at the same line may join it, as for range_loop_element_unframed; out-of-bounds is the other category the read could report
// Pre-registered from record 0060 item 11 (Part D), before the code
#include <cerastes.h>

int walk(void)
    CERASTES_ENSURES(r, r == 1)
{
  int a[4] = {1, 2, 3, 4};
  int *q = a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && q[0] == 1 && q[4] >= 0) CERASTES_DECREASES(3 - i) while (i < 3) {
    i++;
  }
  return q[0];
}
