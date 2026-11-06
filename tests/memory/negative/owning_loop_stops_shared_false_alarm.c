// expect: error leak @ L23
// expect: error invalid-free @ L27
// mode: c23, c++20
// twin: tests/memory/owning_two_owners_loop.c
// Pins: 0052 item 3 Loops, audit 3b (m2_stops_shared.c), a documented false alarm lifted by M4.5d2b2: two owners share a block at the loop's entry and one of them lets go inside the loop (a = NULL): a carrier that is switched off at entry never picks up a new block and nothing exempts it, so the program (correct: b frees the block) is reported: leak at the assignment a = NULL and an invalid-free at free(b) (both kept: one fault, the documented false alarm)
// Pre-registered from audit round 3b (build/m4.5d2b/audit3b.md): the false leak and invalid-free the auditor saw (the lines are the first run's)
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
    }
    i += 1;
  }
  free(b);  // <- here
  return 0;
}
