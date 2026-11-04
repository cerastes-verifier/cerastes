// expect: error overflow @ L11
// mode: c23, c++20
// twin: tests/memory/heap_malloc_free.c
// Pins: 0047 item 2 (audit S1): the front end folds sizeof(int) * 0x4000000000000001u to 4 by wrapping; the argument is still encoded through its original expression, so 0022's no-wrap check fires
// Pre-registered from record 0047 item 2: overflow at the size expression, one error; re-registered after audit round 1 (record 0047 item 2): the size's own checks stay
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *p = (int *)malloc(sizeof(int) * 0x4000000000000001u);  // <- here
  free(p);
  return 0;
}
