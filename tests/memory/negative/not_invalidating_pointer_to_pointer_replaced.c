// expect: error postcondition @ L19
// mode: c23, c++20
// twin: tests/memory/not_invalidating_pointer_to_pointer.c
// Pins: 0058 item 2, audit S8: a marked int **pp whose body replaces *pp (frees the block it points to and stores a fresh one): the implicit ensures *pp == old(*pp) may not hold at the closing brace; the unmarked program with the same body verifies (build/m4.5d2b2d/emul3/p1.c), so the marker is the one reason
// Pre-registered from record 0058 item 2 (audit round 1, S8): one postcondition error at the closing brace of renew
#include <cerastes.h>
#include <stdlib.h>

void renew(int **pp CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
{
  free(*pp);
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 0;
}  // <- here

int use(void)
{
  int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  *a = 1;
  renew(&a);
  free(a);
  return 0;
}
