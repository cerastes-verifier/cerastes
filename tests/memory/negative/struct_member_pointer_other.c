// expect: error uninitialized @ L24
// mode: c23, c++20
// twin: tests/memory/struct_member_pointer.c
// Pins: 0043 item 5: inc(&s.x) says nothing about s.y, which was never written
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && *p == cerastes_old(*p) + 1)
{
  *p += 1;
}

int use_inc(void)
{
  struct S s;
  s.x = 0;
  inc(&s.x);
  return s.y;  // <- here
}
