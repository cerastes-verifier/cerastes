// expect: rejected cerastes-unsupported_annotation @ L18
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/restrict_dropped.c
// Pins: 0044 item 11 (audit round 2): get() has the implicit owns of its static struct local, which has an int * leaf: f calls it, so f holds that atom, and a restrict int * beside it is rejected (the check runs again after the implicit atoms are added) [0064 item 19, question 326: every restrict parameter is now rejected by that one rule, whatever the contract; this pin's row keeps its tag and line and its condition no longer matters]
#include <cerastes.h>

struct S {
  int x;
  int *q;
};

int *get(void)
{
  static struct S s = {0, 0};
  return s.q;
}

void f(int *restrict p)  // <- here
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  int *r = get();
  *p = (r == 0);
}
