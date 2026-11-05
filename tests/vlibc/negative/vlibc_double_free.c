// expect: error invalid-free @ L18
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 11 (trusted contract of vlibc_free, 0030 item 5): the second vlibc_free finds no token
// Pre-registered from record 0050 items 2, 11 (vlibc_free's requires is free's): invalid-free at the second vlibc_free, the first token row of the call
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  *p = 7;
  vlibc_free(p);
  vlibc_free(p);  // <- here
  return 7;
}
