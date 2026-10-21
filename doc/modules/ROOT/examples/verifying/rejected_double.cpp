// expect: rejected cerastes-unsupported_type @ L6
// A construct this version does not verify is rejected with a diagnostic, never approximated.
// tag::source[]
#include <cerastes.h>

double half(double x)
{
  return x / 2;
}
// end::source[]
