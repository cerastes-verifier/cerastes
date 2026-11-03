// expect: rejected cerastes-unsequenced_modification @ L15
// mode: c23, c++20
// twin: tests/memory/struct_value_literal.c
// Pins: 0044 item 5 (P68 L1: no compiler warns): s = (struct S){s.x++, 0}: the element's write to s.x is unsequenced with the store to s.x
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(void)
{
  struct S s = {0, 0};
  s = (struct S){s.x++, 0};  // <- here
  return s.x;
}
