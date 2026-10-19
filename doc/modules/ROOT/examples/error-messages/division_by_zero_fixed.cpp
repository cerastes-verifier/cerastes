// expect: verified
// expect: native-ok
// The same program, fixed.
// tag::source[]
#include <cerastes.h>

unsigned per_item(unsigned total, unsigned count)
    CERASTES_REQUIRES(count != 0u)
{
  return total / count;
}
// end::source[]
