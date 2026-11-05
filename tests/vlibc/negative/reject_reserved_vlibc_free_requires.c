// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a user declaration in C++ has C++ linkage (an ordinary function, decision 0046 item 6); vlibc_free's contract comes from vlibc's <stdlib.h> only
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 8 (0046 item 6): a contract on the reserved name vlibc_free outside vlibc is rejected (a precondition would restrict every caller)
// Pre-registered from record 0050 item 8: rejected annotation_reserved_library at the declaration
#include <cerastes.h>

void vlibc_free(void *ptr) CERASTES_REQUIRES(ptr != 0);  // <- here

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
