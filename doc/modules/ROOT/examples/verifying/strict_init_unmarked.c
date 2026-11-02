// expect: rejected cerastes-uninit_unmarked @ L10
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: a local without an initializer needs CERASTES_UNINIT.
// tag::source[]
#include <cerastes.h>

int f(void)
{
  int x;
  x = 1;
  return x;
}
// end::source[]
