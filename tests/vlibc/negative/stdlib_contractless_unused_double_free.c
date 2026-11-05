// expect: error invalid-free @ L17
// mode: c23, c++20
// twin: tests/vlibc/stdlib_contractless_unused.c
// Pins: 0050 item 6: the same program with a double free still fails at the free (the shim's <stdlib.h> is read in full)
// Pre-registered from record 0050 item 6 and 0047 item 6: invalid-free at the second free
#include <stdlib.h>
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
