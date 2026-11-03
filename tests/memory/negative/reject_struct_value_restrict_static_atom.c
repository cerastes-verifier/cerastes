// expect: rejected cerastes-unsupported_annotation @ L18
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_value_restrict.c
// Pins: 0044 item 11 (audit round 2): get() has the implicit owns of its static struct local, which has an int * leaf: f calls it, so f holds that atom, and a restrict int * beside it is rejected (the check runs again after the implicit atoms are added)
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
