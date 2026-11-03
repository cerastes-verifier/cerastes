// expect: error lifetime @ L23
// mode: c23, c++20
// twin: tests/memory/struct_member_scope.c
// Pins: 0043 item 5: &p->x uses p, whose object ended with its block: lifetime
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void sink(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(cerastes_live(q));

void dangling(void)
{
  struct S *p;
  {
    struct S s = {1, 2};
    p = &s;
  }
  sink(&p->x);  // <- here
}
