// expect: error permission @ L26
// expect: error leak @ L28
// mode: c23, c++20
// twin: tests/memory/owning_two_owners_loop.c
// Pins: 0052 item 3 Loops, audit 3b (m1_becomes_shared.c), a documented false alarm lifted by M4.5d2b2: a second owner takes the first owner's block inside the loop (b = a, b NULL at entry): the block becomes shared there, and the loop keeps one carrier per block: permission at the back edge: owning b takes a block this loop already carries through a (the program is correct: the false alarm is that a block cannot change from one carrier to two inside a loop)
// Pre-registered from audit round 3b (build/m4.5d2b/audit3b.md): permission at the back edge and, as a consequence of the same fault, a leak of b at the return (both kept: a documented false alarm; the lines are the first run's)
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
    }
    i += 1;
  }  // <- here
  free(a);
  return 0;  // <- here
}
