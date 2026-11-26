// expect: error permission @ L26
// expect: error check @ L33
// mode: c++20
// twin: tests/memory/struct_member_array_struct_global_element_value.cpp
// Pins: 0064 item 12 (audit round C2b, finding 3): the by-value pass of gt.v[1] from a function without gt's atom is a `permission` at the call (the leaves are read under the caller's permission: the twin's h holds reads(gt)); and in main the value is gt.v[1].x == 3, so the check of 1 (the other element's x) fails. One reason each
// Registered from audit-C2b probes a/g07 (`permission`: reads `gt.v[1].x` without owning it, at the call line, measured) and a/g02 (`check` at the check, measured), laid out in this file, before the first run of this file; the rows' lines are inferred from the layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

static struct T gt = {{{1, 2}, {3, 4}}, 5};

int fp(struct P a)
    CERASTES_ENSURES(r, r == a.x);

int no_atom(void)
{
  int b = fp(gt.v[1]);  // <- here
  return b;
}

int main()
{
  int b = fp(gt.v[1]);
  CERASTES_CHECK(b == 1);  // <- here
  return 0;
}
