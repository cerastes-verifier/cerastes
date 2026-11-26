// expect: error precondition @ L30
// expect: error check @ L38
// mode: c23, c++20
// twin: tests/memory/struct_member_array_struct_param.c
// Pins: 0064 item 12 (audit round C2, finding 3, the by-value PARAMETER): use_bad passes a structure whose m is 9 to f, whose requires is `t.m == 8`: the precondition fails at the call; use_bad2 passes a structure whose n is 8 and checks that the result is 1, but the callee's ensures ties it to the caller's n. One reason each: the argument value is the caller's, leaf by leaf
// Registered from audit-C2 probe c04 (use_bad: precondition at the call; use_bad2: check at its line; both measured on snapshot a044aff629b8); the rows' lines are inferred from this layout
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T2 {
  int n;
  struct P v[2];
  int m;
};

int f(struct T2 t)
    CERASTES_REQUIRES(t.m == 8)
    CERASTES_ENSURES(r, r == t.n)
{
  return t.n;
}

int use_bad(void)
{
  struct T2 t = {1, {{8, 8}, {8, 8}}, 9};
  int w = f(t);  // <- here
  return w;
}

int use_bad2(void)
{
  struct T2 t = {8, {{8, 8}, {8, 8}}, 8};
  int w = f(t);
  CERASTES_CHECK(w == 1);  // <- here
  return w;
}
