// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a definition in C++ has C++ linkage (an ordinary function, decision 0046 item 6)
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0046 item 6 (question 117; C23 7.1.3) and 0047 item 1: a user definition of the library's malloc is rejected
// Pre-registered from record 0047 item 1: rejected annotation_reserved_library at the definition (the size_t parameter is unsigned long on both platforms)
#include <cerastes.h>

void *malloc(unsigned long size)  // <- here
{
  (void) size;
  return 0;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
