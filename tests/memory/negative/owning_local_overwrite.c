// expect: error leak @ L17
// mode: c23, c++20
// twin: tests/memory/owning_local_free.c
// Pins: 0052 item 3: reassigning an owning local while it holds a block that nothing else owns: the reassignment row on the old value fails at the assignment
// Pre-registered from record 0052 item 3: leak at the assignment
#include <cerastes.h>
#include <stdlib.h>

int overwrite(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_OWNING int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  p = (int *)malloc(sizeof(int));  // <- here
  free(p);
  return 0;
}
