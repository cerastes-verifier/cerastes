// expect: rejected cerastes-unsupported_expression @ L13
// twin: tests/memory/struct_literal_value_cpp.cpp
// Pins: 0044 item 7 (flip of reject_struct_compound_literal.c, the c++20 half): the address of a struct compound literal in C++
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int compound()
{
  struct S *p = &(struct S){1, 2};  // <- here
  return p->x;
}
