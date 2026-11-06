// expect: error permission @ L26
// mode: c23, c++20
// twin: tests/memory/owning_two_owners_loop.c
// Pins: 0052 item 3 Loops (audit 3 B1): the same two owners: the block is freed through b and then written through a: a user error (the other owner is dangling): permission at the write
// Pre-registered from audit round 3 (build/m4.5d2b/audit3.md) S2 B1: permission at the write through the dangling owner (could be lifetime: question 160)
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
    *a = i;
    *b = i;
    i += 1;
  }
  free(b);
  *a = 1;  // <- here
  return 0;
}
