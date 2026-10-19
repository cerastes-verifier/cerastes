// expect: verified
// expect: native-ok
// A contract on a declaration is trusted; the caller is verified against it.
// tag::source[]
#include <cerastes.h>

int next_id(int last)
    CERASTES_REQUIRES(0 <= last && last < 1000000)
    CERASTES_ENSURES(r, r == last + 1);

int two_ids(int last)
    CERASTES_REQUIRES(0 <= last && last < 1000)
    CERASTES_ENSURES(r, r == last + 2)
{
  return next_id(next_id(last));
}
// end::source[]
