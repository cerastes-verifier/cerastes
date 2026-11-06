// expect: error leak @ L23
// mode: c23, c++20
// twin: tests/memory/owning_local_dangling.c
// Pins: 0052 item 3 (audit S5): the same without the final free: k holds a block and nothing hands it back: exactly one leak, at the return (a, freed, adds none)
// Pre-registered from record 0052 item 3 (audit round 1, S12): leak at the return, once
#include <cerastes.h>
#include <stdlib.h>

int again(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  *a = 1;
  free(a);
  CERASTES_OWNING int *k = (int *)malloc(sizeof(int));
  if (k == NULL) {
    return -1;
  }
  *k = 2;
  return 0;  // <- here
}
