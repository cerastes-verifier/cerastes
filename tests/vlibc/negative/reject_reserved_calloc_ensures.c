// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a user declaration in C++ has C++ linkage (an ordinary function, decision 0046 item 6)
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0046 item 6 and 0047 item 1: a user contract on the reserved name `calloc` is rejected
// Pre-registered from record 0047 item 1: rejected annotation_reserved_library at the declaration
#include <cerastes.h>

void *calloc(unsigned long n, unsigned long size) CERASTES_ENSURES(r, r == 0);  // <- here

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
