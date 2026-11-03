// expect: error lifetime @ L16
// mode: c23, c++20
// twin: tests/memory/struct_live_member_address.c
// Pins: 0043 item 5: &p->x and &p->y are arithmetic on p, whose object ended with its block (the member blocks' lengths and p's bounds are known from the allocation): lifetime only
#include <cerastes.h>

struct S { int x; int y; };

int dangling(void)
{
  struct S *p;
  {
    struct S s = {0, 0};
    p = &s;
  }
  return &p->x == &p->y;  // <- here
}
