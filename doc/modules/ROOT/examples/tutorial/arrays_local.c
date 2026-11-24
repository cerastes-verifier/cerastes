// expect: verified
// expect: native-ok
// mode: c23, c++20
// A local array and an index: every index must be inside the array.
// tag::source[]
#include <cerastes.h>

int mark_and_read(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 7)
{
  int a[3] = {0, 0, 0};
  a[i] = 7;
  return a[i];
}
// end::source[]
