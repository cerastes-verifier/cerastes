// expect: verified
// expect: native-ok
// mode: c23, c++20
// A member marked ref_to_uninit: the pointer it holds may point to an object without a value.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

int main(void)
{
  int x CERASTES_UNINIT;
  struct cursor c = { &x, 0 };
  *c.target = 4;
  return c.n + *c.target - 4;
}
// end::source[]
