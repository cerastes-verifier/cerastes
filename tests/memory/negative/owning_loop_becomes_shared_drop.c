// expect: error leak @ L25
// expect: error leak @ L30
// mode: c23, c++20
// twin: tests/memory/owning_loop_becomes_shared.c
// Pins: 0054 item 3: the same loop that lets go of the shared block: a = NULL while b holds it is fine, b = NULL then drops the last owner: the reassignment row
// Pre-registered from record 0054 item 3 (S39 d2c's drop at a reassignment) and owner question 185 (every reported line is listed, as question 183): leak at b = NULL, and leak at the return (b's value after the loop is unconstrained: no value set, since a is also written, so b's exit row fires)
#include <cerastes.h>
#include <stdlib.h>

int main(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return 1;
  }
  *a = 0;
  CERASTES_OWNING int *b = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    if (i == 2 && b == NULL) {
      b = a;
      a = NULL;
      b = NULL;  // <- here
    }
    i += 1;
  }
  free(a);
  return 0;  // <- here
}
