// expect: error leak @ L24
// expect: error leak @ L25
// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/owning_loop_stops_shared.c
// Pins: 0054 item 3: the loop lets go of both owners of the shared block: b = NULL after a = NULL drops the last owner. The L24 expectation (leak at a = NULL) is a FALSE ALARM pinned by owner question 185: in the real program a = NULL does not leak (b still owns the block there), so a later gain in precision that removes it is not a regression: re-expect the line then
// Pre-registered from record 0054 item 3 (S39 d3b) and owner question 185 (every reported line is listed, as question 183): leak at a = NULL (the reassignment row, b = a being a plain copy of an owner whose value the loop then clears: both owners are written), leak at b = NULL, and leak at the return (a's exit row)
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  CERASTES_OWNING int *b = a;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (a != NULL) {
      a = NULL;  // <- here
      b = NULL;  // <- here
    }
    i += 1;
  }
  free(b);
  return 0;  // <- here
}
