// expect: verified
// expect: native-ok
// The same program, fixed.
// tag::source[]
#include <cerastes.h>

int increment(int x)
    CERASTES_REQUIRES(x < 2147483647)
{
  return x + 1;
}
// end::source[]
