// expect: error invalid-free @ L16
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_calloc_zero.c
// Pins: 0050 items 3, 5 (0048 item 4; audit round 1 S4): a block of vlibc_calloc freed with the platform's free
// Pre-registered from record 0050 item 5: invalid-free at the free call, one error
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)vlibc_calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  free(p);  // <- here
  return 0;
}
