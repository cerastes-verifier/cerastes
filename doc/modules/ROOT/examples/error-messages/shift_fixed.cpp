// expect: verified
// expect: native-ok
// The same program, fixed.
// tag::source[]
#include <cerastes.h>

unsigned bit(unsigned n)
    CERASTES_REQUIRES(n < 32u)
{
  return 1u << n;
}
// end::source[]
