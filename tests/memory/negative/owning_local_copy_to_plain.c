// expect: error leak @ L20
// mode: c23, c++20
// twin: tests/memory/owning_local_handoff.c
// Pins: 0052 item 3: the pointer is copied to a local that is not an owner and the owner's scope ends: the block is still held when t's lifetime ends and no owner remains holding it (a plain local is not an owner)
// Pre-registered from record 0052 item 3: leak at the closing brace of the inner block
#include <cerastes.h>
#include <stdlib.h>

int copy_to_plain(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  int *keeper = NULL;
  {
    CERASTES_OWNING int *t = (int *)malloc(sizeof(int));
    if (t == NULL) {
      return -1;
    }
    *t = 3;
    keeper = t;
  }  // <- here
  free(keeper);
  return 0;
}
