// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 items 3, 5: a by-value struct parameter is the tuple of its members (formals p_s__0_m0 and p_s__0_m1, each with its range), s.x in the contract is the entry value, and the caller reads every member of its struct before the call
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int get(struct S s)
    CERASTES_REQUIRES(s.x > 0)
    CERASTES_ENSURES(r, r == s.x)
{
  return s.x;
}

int use_get(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {3, 4};
  return get(s);
}
