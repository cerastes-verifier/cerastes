// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a definition in C++ has C++ linkage (an ordinary function, decision 0046 item 6)
// twin: tests/vlibc/vlibc_alloc_roundtrip.c
// Pins: 0050 item 8 (0048 item 5): a user definition of vlibc's vlibc_malloc is rejected: the name is reserved because vlibc declares it with a contract
// Pre-registered from record 0050 item 8: rejected annotation_reserved_library at the definition (the size_t parameter is unsigned long on both platforms)
#include <cerastes.h>

void *vlibc_malloc(unsigned long size)  // <- here
{
  (void) size;
  return 0;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
