// expect: rejected cerastes-unsupported_expression @ L14
// c-only: _Atomic does not exist in C++20 (decision 0044 item 7)
// twin: tests/memory/struct_literal_member.c
// Pins: 0044 item 7 (audit round 2): an _Atomic-qualified struct compound literal as an initializer
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(void)
{
  struct S s = (_Atomic struct S){1, 2};  // <- here
  return s.x;
}
