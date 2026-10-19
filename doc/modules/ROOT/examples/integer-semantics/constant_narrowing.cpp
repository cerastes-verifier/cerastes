// expect: error conversion @ L8
// A constant that does not fit a signed type is a conversion error.
// tag::source[]
#include <cerastes.h>

signed char too_big(void)
{
  return 300;
}
// end::source[]
