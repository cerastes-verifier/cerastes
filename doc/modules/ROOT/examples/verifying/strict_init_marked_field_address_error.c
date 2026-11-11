// expect: rejected cerastes-uninit_ref_initialized @ L17
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: a marked member receives a pointer that may point to an object with a value.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

void point(struct cursor *c, int *x)
    CERASTES_REQUIRES(cerastes_owns(*c))
    CERASTES_ENSURES(cerastes_owns(*c))
{
  c->target = x;
}
// end::source[]
