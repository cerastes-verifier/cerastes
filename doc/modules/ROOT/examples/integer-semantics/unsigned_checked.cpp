// expect: verified
// expect: native-ok
// An unsigned operation is checked like a signed one: the requires keeps the sum in range.
// tag::source[]
#include <cerastes.h>

unsigned next(unsigned x)
    CERASTES_REQUIRES(x < 4294967295u)
    CERASTES_ENSURES(r, r == x + 1u)
{
  return x + 1u;
}
// end::source[]

unsigned next_of_max_minus_one(void)
    CERASTES_ENSURES(r, r == 4294967295u)
{
  return next(4294967294u);
}
