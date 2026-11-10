// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/not_invalidating_pointer_to_pointer.c
// Pins: 0058 item 2 (as revised; 0054 item 2's given shape): the ensures has owns_block(*pp) and the requires has none, and the body stores a fresh block into *pp: a block site rooted at the marked parameter with no partner. Without the marker this verifies (the written form is build/m4.5d2b2d/emul/k2.c), so the marker is the one reason
// Pre-registered from record 0058 item 2 (as revised): rejected unsupported_annotation at the parameter; a bodiless version is rejected today at the ensures clause by 0052 item 2a (build/m4.5d2b2d/emul/k_give_block.c), so the body keeps this pin about the marker alone
#include <cerastes.h>
#include <stdlib.h>

void give(int **pp CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns_block(*pp))
{
  *pp = (int *)malloc(sizeof(int));
  if (*pp == NULL) {
    abort();
  }
  **pp = 0;
}

int main(void)
{
  return 0;
}
