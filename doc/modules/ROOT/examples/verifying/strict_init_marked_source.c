// expect: rejected cerastes-uninit_ref_initialized @ L11
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: a marked local receives the address of an object that has a value.
// tag::source[]
#include <cerastes.h>

int f(void)
{
  int x = 5;
  CERASTES_REF_TO_UNINIT int *p = &x;
  return *p;
}
// end::source[]
