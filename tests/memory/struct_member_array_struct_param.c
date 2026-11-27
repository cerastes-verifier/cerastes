// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 12 (audit round C2, finding 3, a by-value PARAMETER that holds a member array of structures: 6 leaves, under the limit of 7): the callee's requires reads the member m of the value and its ensures the member n; at each call the caller's structure is the argument value leaf by leaf (the four element leaves are copied too), so the callee's precondition is checked on the caller's m and the result is tied to the caller's n. The twin negative/struct_member_array_struct_param_wrong.c passes a structure whose m violates the requires and checks a result that is not the caller's n
// Registered from audit-C2 probe c04 (use_ok and its two sibling functions, measured on snapshot a044aff629b8: f's precondition and the checks at their lines; the designed-false `w == 0` of use_ok is dropped; the file's use_other is c04's use_bad2 with the check the callee's contract implies), before the first run on this build; the compound-literal form (c15's use2) is not here: compound literals are not C++
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

int use_ok(void)
    CERASTES_ENSURES(r, r == 1)
{
  struct T2 t = {1, {{0, 0}, {0, 0}}, 8};
  int w = f(t);
  CERASTES_CHECK(w == 1);
  return w;
}

int use_other(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct T2 t = {8, {{8, 8}, {8, 8}}, 8};
  int w = f(t);
  CERASTES_CHECK(w == 8);
  return w;
}
