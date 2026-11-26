// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0064 item 12 (audit round C2b, finding 3: the C++ path): an element of the member array of a GLOBAL structure passed by value is accepted in C++ and its leaves are read under the caller's permission: in main with gt known (b is gt.v[1].x == 3, c is gt.v[0].x == 1), and in a function that holds reads(gt). The twin negative/struct_member_array_struct_global_element_value_wrong.cpp checks the wrong element and calls from a function without gt's atom. C rejects the same call (negative/struct_member_array_struct_reject_global_element_value_c.c)
// Registered from audit-C2b probes h01 (both checks measured verified in C++, its designed-false `0 == 1` dropped) and a/g07 (`fp(gt.v[1])` from a function without gt's atom: `permission`); `h` with reads(gt) has no probe of exactly this form (g04's symbolic-index form timed out, g05's fr is the neighbour), unmeasured
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

int h(void)
    CERASTES_REQUIRES(cerastes_reads(gt))
    CERASTES_ENSURES(cerastes_reads(gt))
{
  return fp(gt.v[1]);
}

int main()
{
  int b = fp(gt.v[1]);
  CERASTES_CHECK(b == 3);
  int c = fp(gt.v[0]);
  CERASTES_CHECK(c == 1);
  return b + c - 4;
}
