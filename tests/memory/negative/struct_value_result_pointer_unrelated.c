// expect: error lifetime @ L26
// mode: c23, c++20
// twin: tests/memory/struct_value_result_pointer.c
// Pins: 0044 item 4: the ensures does not relate r.p to the arguments: copying the result's pointer member into t is a use of a pointer that may be dangling, lifetime at that copy
#include <cerastes.h>

struct P {
  int *p;
  int v;
};

struct P pick(int *a, int *b, int c)
    CERASTES_REQUIRES(cerastes_owns(*a) && cerastes_owns(*b))
    CERASTES_ENSURES(r, cerastes_owns(*a) && cerastes_owns(*b) && r.v == 0)
{
  struct P s;
  s.v = 0;
  s.p = c != 0 ? a : b;
  return s;
}

void use_pick(void)
{
  int c1 = 1;
  int c2 = 2;
  struct P t = pick(&c1, &c2, 1);  // <- here
  CERASTES_CHECK(t.v == 0);
}
