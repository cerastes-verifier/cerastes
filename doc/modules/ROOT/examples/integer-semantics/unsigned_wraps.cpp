// expect: verified
// expect: native-ok
// Unsigned arithmetic is modular: no error, and the contract can say so.
// tag::source[]
#include <cerastes.h>

unsigned wrap(unsigned x)
    CERASTES_REQUIRES(x == 4294967295u)
    CERASTES_ENSURES(r, r == 0u)
{
  return x + 1u;
}
// end::source[]
