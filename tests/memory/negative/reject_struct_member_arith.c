// expect: rejected cerastes-unsupported_expression @ L15
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 5: arithmetic on a member pointer is rejected (M4.5e2)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int past_member(void)
{
  struct S s = {1, 2};
  int *q = &s.x + 1;  // <- here
  return q == &s.y;
}
