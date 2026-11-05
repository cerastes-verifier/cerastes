// expect: error uninitialized @ L17
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 2 (0047 items 2, 8): vlibc_malloc's view is ref_to_uninit like malloc's, so its cell holds no value and a read before the first write is `uninitialized`
// Pre-registered from record 0050 item 2 (audit round 1 S4): uninitialized at the read, as heap_read_before_write.c for malloc
#include <vlibc/stdlib.h>
#include <cerastes.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r <= 2147483647)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  if (p == NULL) {
    return -1;
  }
  int v = *p;  // <- here
  vlibc_free(p);
  return v;
}
