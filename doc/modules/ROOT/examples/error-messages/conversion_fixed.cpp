// expect: verified
// expect: native-ok
// The same program, fixed.
// tag::source[]
#include <cerastes.h>

signed char to_small(int x)
    CERASTES_REQUIRES(0 <= x && x <= 127)
{
  return (signed char)x;
}
// end::source[]
