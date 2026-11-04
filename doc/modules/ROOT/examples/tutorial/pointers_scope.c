// expect: verified
// expect: native-ok
// mode: c23, c++20
// A pointer to a block-scoped local is used only while the local exists.
// tag::source[]
#include <cerastes.h>
#include <limits.h>

int use_in_block(int a)
    CERASTES_REQUIRES(a < INT_MAX)
    CERASTES_ENSURES(r, r == a + 1)
{
  int result;
  {
    int x = a;
    int *p = &x;
    *p = *p + 1;
    result = x;
  }
  return result;
}
// end::source[]
