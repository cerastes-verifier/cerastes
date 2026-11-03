// expect: error termination @ L33
// mode: c23, c++20
// twin: tests/memory/struct_decreases_member.c
// Pins: 0044 item 10: the copy keeps s.x, so decreases(s.x) does not decrease: termination at the call
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int countdown(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= p->x && p->x <= 10)
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0 && p->x == 0)
    CERASTES_DECREASES(p->x)
{
  if (p->x == 0) {
    return 0;
  }
  p->x -= 1;
  return countdown(p);
}

int down(struct S s)
    CERASTES_REQUIRES(0 <= s.x && s.x <= 10)
    CERASTES_ENSURES(r, r == 0)
    CERASTES_DECREASES(s.x)
{
  if (s.x == 0) {
    return 0;
  }
  struct S t = {s.x, s.y};
  return down(t);  // <- here
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct S s = {5, 1};
  int a = countdown(&s);
  int b = down(s);
  return a + b;
}
