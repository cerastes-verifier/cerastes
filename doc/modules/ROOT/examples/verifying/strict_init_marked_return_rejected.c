// expect: rejected cerastes-uninit_ref_initialized @ L13
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: a function marked ref_to_uninit returns a pointer that may point to an object with a value.
// tag::source[]
#include <cerastes.h>
#include <stdlib.h>

CERASTES_REF_TO_UNINIT int *pass_on(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, r == NULL || cerastes_owns_block(r))
{
  return p;
}
// end::source[]
