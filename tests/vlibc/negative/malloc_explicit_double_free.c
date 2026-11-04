// expect: error invalid-free @ L17
// mode: c23, c++20
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0047 item 6 and 0046 item 9: a twin of free's trusted requires through the explicit header
// Pre-registered from record 0047 item 6: invalid-free at the second free
#include <vlibc/stdlib.h>
#include <cerastes.h>

int roundtrip(void)
{
  int *p = (int *)malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  free(p);
  free(p);  // <- here
  return 7;
}
