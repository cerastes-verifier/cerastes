// expect: rejected cerastes-unsupported_expression @ L19
// c-only: qualified compound literals are C only here: in C++ a volatile object has no copy into a plain struct (decision 0044 item 7)
// twin: tests/memory/struct_literal_member.c
// Pins: 0044 item 7 (audit round 2): a volatile-qualified struct compound literal as an argument
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int g(struct S s)
{
  return s.x;
}

int f(void)
{
  return g((volatile struct S){1, 2});  // <- here
}
