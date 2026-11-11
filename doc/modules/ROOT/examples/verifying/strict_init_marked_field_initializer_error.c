// expect: rejected cerastes-uninit_ref_initialized @ L16
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: an initializer gives a marked member the address of an initialized object.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

int main(void)
{
  int x = 0;
  struct cursor c = { &x, 1 };
  return c.n;
}
// end::source[]
