// expect: error invalid-free @ L17
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 items 3, 5 (0048 item 4): a platform block freed with vlibc_free (vlibc's heap is another heap)
// Pre-registered from record 0050 item 5: invalid-free at the vlibc_free call, one error (the value row ends its path)
#include <vlibc/stdlib.h>
#include <cerastes.h>

int f(void)
{
  int *q = (int *)malloc(sizeof(int));
  if (q == NULL) {
    return -1;
  }
  *q = 2;
  vlibc_free(q);  // <- here
  return 0;
}
