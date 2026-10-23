// expect: error conversion @ L8
// -1 does not fit unsigned int: a conversion error at the initializer, not UINT_MAX.
// tag::source[]
#include <cerastes.h>

unsigned all_ones(void)
{
  unsigned u = -1;
  return u;
}
// end::source[]
