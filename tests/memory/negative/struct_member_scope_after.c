// expect: error lifetime @ L21
// mode: c23, c++20
// twin: tests/memory/struct_member_scope.c
// Pins: 0043 item 5: after the block p == &s.y uses a pointer to a member of t, whose lifetime ended
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int inner(void)
{
  int *p;
  struct S s = {1, 2};
  {
    struct S t = {5, 6};
    p = &t.y;
    s.x = *p;
  }
  if (p == &s.y) {  // <- here
    s.x = 0;
  }
  return s.x;
}
