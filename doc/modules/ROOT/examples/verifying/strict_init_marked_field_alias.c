// expect: verified
// expect: native-ok
// mode: c23, c++20
// verify-options: --strict-init
// What --strict-init cannot see: a store through a pointer to the member is not checked.
// tag::source[]
#include <cerastes.h>

struct cursor {
  int *target CERASTES_REF_TO_UNINIT;
  int n;
};

int main(void)
{
  int x = 5;
  struct cursor c = { 0, 1 };
  int **pp = &c.target;
  *pp = &x;
  return c.n - 1;
}
// end::source[]
