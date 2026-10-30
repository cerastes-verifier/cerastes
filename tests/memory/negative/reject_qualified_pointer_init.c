// expect: rejected cerastes-unsupported_expression @ L13
// c-only: C's int ** to const int *const * conversion has no EDG diagnostic (P62 H2); C++ accepts it (qualification conversion, ptrptr_const_both.cpp)
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (P62 H2): in C an int ** converted to const int *const * is rejected by the encoder (GCC 16 errors, clang warns)
#include <cerastes.h>
#include <stddef.h>

int qualified_init(void)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  const int *const *cq = pp;
  return cq == NULL;
}
