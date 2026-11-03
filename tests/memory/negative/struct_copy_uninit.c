// expect: error uninitialized @ L16
// mode: c23, c++20
// twin: tests/memory/struct_copy.c
// Pins: 0043 item 7 (question 92): a copy reads every member, and s.y was never written: uninitialized at the copy, naming the member
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int use_copy(void)
{
  struct S s;
  s.x = 1;
  struct S t = s;  // <- here
  return t.x;
}
