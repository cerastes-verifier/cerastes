// expect: error overflow @ L8
// Without the bound, x + 1u at UINT_MAX does not fit unsigned int: cerastes does not wrap.
// tag::source[]
#include <cerastes.h>

unsigned next(unsigned x)
{
  return x + 1u;
}
// end::source[]
