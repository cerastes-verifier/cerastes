// expect: rejected cerastes-unsupported_expression @ L9
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit B2, M4.6): the address of a const-qualified compound literal: C23 6.5.2.5p7 lets two such literals share one object
#include <cerastes.h>

int f(void)
{
  const int *p = &(const int){1};  // <- here
  return *p;
}
