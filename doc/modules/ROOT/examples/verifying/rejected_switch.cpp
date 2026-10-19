// expect: rejected cerastes-unsupported_statement @ L8
// A construct M3 does not verify is rejected with a diagnostic, never approximated.
// tag::source[]
#include <cerastes.h>

int classify(int x)
{
  switch (x) {
  case 0:
    return 10;
  default:
    return 20;
  }
}
// end::source[]
