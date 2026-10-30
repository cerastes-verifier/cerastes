// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 4 and 0020 E3: int s = x && bump(&x): the read of x is a snapshot taken before the branch (left operand of &&), the call runs only when it is not 0
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int and_call(int v)
    CERASTES_REQUIRES(0 <= v && v < 100)
    CERASTES_ENSURES(r, r == (v != 0 ? 1 : 0))
{
  int x = v;
  int s = x && bump(&x);
  CERASTES_CHECK(x == (v != 0 ? v + 1 : v));
  return s;
}
