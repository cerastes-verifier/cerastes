// expect: rejected cerastes-unsupported_expression @ L14
// c-only: qualified compound literals are C only here: in C++ a volatile object has no copy into a plain struct (decision 0044 item 7)
// twin: tests/memory/struct_literal_member.c
// Pins: 0044 item 7 (audit round 2): a volatile-qualified struct compound literal as an initializer: a volatile object is M8, rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(void)
{
  struct S s = (volatile struct S){1, 2};  // <- here
  return s.x;
}
