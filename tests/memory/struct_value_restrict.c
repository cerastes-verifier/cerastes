// expect: verified
// expect: native-ok
// c-only: compound literal objects and restrict do not exist in C++ (decision 0044 items 7 and 11)
// Pins: 0044 item 11: a restrict int * beside a by-value struct whose members are all scalars is accepted (no int * can be formed from them)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int f(int *restrict p, struct S s)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == s.x && r == s.y)
{
  *p = s.x;
  return s.y;
}

int use(void)
    CERASTES_ENSURES(r, r == 2)
{
  int a = 0;
  struct S s = {1, 2};
  return f(&a, s);
}
