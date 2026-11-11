// expect: rejected cerastes-uninit_ref_initialized @ L15
// mode: c23, c++20
// verify-options: --strict-init
// Rejected by --strict-init: a global's marked member cannot start as the address of an object, which always holds a value.
// tag::source[]
#include <cerastes.h>

int x;

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

struct cursor g = { &x, 1 };

int main(void)
{
  return 0;
}
// end::source[]
