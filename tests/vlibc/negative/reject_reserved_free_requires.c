// expect: rejected cerastes-annotation_reserved_library @ L8
// c-only: without <stdlib.h> a user declaration in C++ has C++ linkage (an ordinary function, decision 0046 item 6); free's contract comes from vlibc's <stdlib.h> only
// twin: tests/vlibc/malloc_explicit.c
// Pins: 0046 item 6 (question 115) and 0047 item 1: a contract on the reserved name `free` outside vlibc is rejected (a precondition would restrict every caller; the unit has no <stdlib.h>)
// Pre-registered from record 0047 item 1 (the reserved table gains malloc, calloc and free): rejected annotation_reserved_library at the declaration
#include <cerastes.h>

void free(void *ptr) CERASTES_REQUIRES(ptr != 0);  // <- here

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
