// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 9: *r.q of a struct result, framed like *result by ensures(r.q == p && owns(*p) && *r.q == 1); the caller reads *s.q after the call
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

struct P mk(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, r.q == p && cerastes_owns(*p) && *r.q == 1)
{
  *p = 1;
  struct P s = {p};
  return s;
}

int use_mk(void)
    CERASTES_ENSURES(r, r == 1)
{
  int x = 0;
  struct P s = mk(&x);
  return *s.q;
}
