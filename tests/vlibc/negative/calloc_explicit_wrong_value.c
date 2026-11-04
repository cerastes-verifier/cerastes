// expect: error check @ L16
// mode: c23, c++20
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0047 item 1 (0046 item 9): a twin of calloc's trusted ensures (zeroed): the value is 0
// Pre-registered from record 0047 item 1: check at the CERASTES_CHECK
#include <vlibc/stdlib.h>
#include <cerastes.h>

int zeroed(void)
{
  int *p = (int *)calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;
  CERASTES_CHECK(v != 0);  // <- here
  free(p);
  return v;
}
