// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// A global structure's marked member starts as a null pointer.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

struct cursor g = { 0, 1 };

int main(void)
{
  return g.n > 0 ? 0 : 1;
}
// end::source[]
