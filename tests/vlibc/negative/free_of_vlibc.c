// expect: error invalid-free @ L17
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 items 3, 5 (0048 item 4): a vlibc block freed with the platform's free
// Pre-registered from record 0050 item 5: invalid-free at the free call, one error (the value row ends its path)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 1;
  free(p);  // <- here
  return 0;
}
