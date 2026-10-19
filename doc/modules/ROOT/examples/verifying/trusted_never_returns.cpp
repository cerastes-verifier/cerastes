// expect: verified
// expect: native-ok
// A contract on a declaration is trusted, its termination included: ensures(false) makes any ensures of the caller hold.
// tag::source[]
#include <cerastes.h>

int never_returns(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, false);

int absurd(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(r, r == 42)
{
  return never_returns(x);
}
// end::source[]
