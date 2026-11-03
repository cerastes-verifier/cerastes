// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 5: passing *p by value reads every member of *p (reads(*p) is enough: a copy source, 0043 item 7), the base's null check once before any member
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int first(struct S s)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int via_ptr(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == p->x)
{
  return first(*p);
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct S s = {4, 5};
  return via_ptr(&s);
}
