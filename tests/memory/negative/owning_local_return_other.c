// expect: error leak @ L22
// mode: c23, c++20
// twin: tests/memory/owning_local_return.c
// Pins: 0052 item 3: the function returns b while the owning local a still holds its block: a is neither b nor freed, so its lifetime-end row at the return fails
// Pre-registered from record 0052 item 3: leak at the return of the other value
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *second(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a == NULL) {
    return NULL;
  }
  *a = 1;
  CERASTES_OWNING int *b = (int *)malloc(sizeof(int));
  if (b == NULL) {
    free(a);
    return NULL;
  }
  *b = 2;
  return b;  // <- here
}
