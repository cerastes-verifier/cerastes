// expect: verified
// expect: native-ok
// mode: c23, c++20
// The value of a comma exists only after its left operand has run: validate ends the program unless y > 0, so the precondition of h and the divisor are checked after it.
// tag::source[]
#include <cerastes.h>

// Returns only when y > 0 (a trusted contract: the program ends otherwise).
int validate(int y)
    CERASTES_ENSURES(r, y > 0);

int h(int a)
    CERASTES_REQUIRES(a > 0)
    CERASTES_ENSURES(r, r == 100 / a)
{
  return 100 / a;
}

int checked(int y)
    CERASTES_ENSURES(r, 0 <= r && r <= 100)
{
  return h((validate(y), y));
}

int ratio(int y)
    CERASTES_ENSURES(r, y > 0 && r == 100 / y)
{
  return 100 / (validate(y), y);
}
// end::source[]
