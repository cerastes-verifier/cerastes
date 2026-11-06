// expect: rejected cerastes-uninit_ref_initialized @ L15
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: a null pointer is not the address of an object without a value.
// tag::source[]
#include <cerastes.h>
#include <stddef.h>

void fill(int *p CERASTES_REF_TO_UNINIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p));

void f(void)
{
  fill(NULL);
}
// end::source[]
