// expect: error null-dereference @ L14
// mode: c23
// c-only: vlibc_malloc creates no C++ objects (record 0050 item 7)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 2 (0047 item 2): vlibc_malloc's contract is `r == NULL || owns_block(r)`, so the result may be null and a write through it without a test is a null dereference
// Pre-registered from record 0050 item 2 (audit round 1 S4): null-dereference at the write, nothing else, as heap_no_null_test.c for malloc
#include <vlibc/stdlib.h>
#include <cerastes.h>

int roundtrip(void)
    CERASTES_ENSURES(r, r == 7 || r == -1)
{
  int *p = (int *)vlibc_malloc(sizeof(int));
  *p = 7;  // <- here
  vlibc_free(p);
  return 7;
}
