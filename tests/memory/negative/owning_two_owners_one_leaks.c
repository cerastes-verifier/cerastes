// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_two_owners.c
// Pins: 0052 item 3 (audit S6): one of the two owners is not freed: exactly one leak, at the return (the freed one adds none and the failing ending row reports its token once)
// Pre-registered from record 0052 item 3 (audit round 1, S12): leak at the return, once
#include <cerastes.h>
#include <stdlib.h>

int two(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return -1;
  }
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return -1;
  }
  free(a);
  return 0;  // <- here
}
