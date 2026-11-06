// expect: error leak @ L19
// mode: c23, c++20
// twin: tests/memory/owning_for_init.c
// Pins: 0052 item 3 (audit round 2 N5): the same loop without the free: the break leaves the for's scope with the owner's block held: leak at the for's closing brace (the break jumps to the scope's end row, as the switch twin; set from the break's line under question 160)
// Pre-registered from audit round 2 (build/m4.5d2b/audit2.md) N5: leak at the for's closing brace
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  int n = 1;
  CERASTES_INVARIANT(0 <= n && n <= 1)
  CERASTES_DECREASES(n)
  for (CERASTES_OWNING int *v = (int *)malloc(sizeof(*v)); n > 0; n--) {
    if (v != NULL) {
      *v = 1;
    }
    break;
  }  // <- here
  return 0;
}
