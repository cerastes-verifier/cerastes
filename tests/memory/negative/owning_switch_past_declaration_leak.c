// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_switch_past_declaration.c
// Pins: 0052 item 3 (audit S4): the case that allocates does not free: the break leaves the owner's scope with the block held: leak at the break
// Pre-registered from record 0052 item 3 (audit round 1, S12): leak at the break
#include <cerastes.h>
#include <stdlib.h>

int sw(int c)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  switch (c) {
    CERASTES_OWNING int *p;
  case 0:
    p = (int *)malloc(sizeof(int));
    if (p == NULL) {
      return -1;
    }
    *p = 1;
    break;
  default:
    break;
  }  // <- here
  return 0;
}
