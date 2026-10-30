// expect: rejected cerastes-unsupported_expression @ L18
// c-only: C's int ** to const int *const * conversion has no EDG diagnostic (P62 H2); C++ accepts it (qualification conversion, ptrptr_const_both.cpp)
// twin: tests/memory/ptrptr_const_middle.c
// Pins: 0040 item 2 (P62 H2): the same conversion of an argument to a const int *const * parameter
#include <cerastes.h>
#include <stddef.h>

int take(const int *const *q)
{
  return q == NULL;
}

int qualified_arg(void)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  return take(pp);
}
