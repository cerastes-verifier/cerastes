// expect: error check @ L17
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_calloc_zero.c
// Pins: 0050 items 2, 11 (trusted contract of vlibc_calloc): the value is zero, so v != 0 is false
// Pre-registered from record 0050 item 2 (vlibc_calloc's ensures is calloc's): check at the CERASTES_CHECK
#include <vlibc/stdlib.h>
#include <cerastes.h>

int zeroed(void)
{
  int *p = (int *)vlibc_calloc(1, sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;
  CERASTES_CHECK(v != 0);  // <- here
  vlibc_free(p);
  return v;
}
