// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: p is compared inside the block, while x exists.
// tag::source[]
#include <cerastes.h>

int compare_in_block(int a)
    CERASTES_ENSURES(r, r == 1)
{
  int y = 0;
  int *p = &y;
  int found = 0;
  {
    int x = a;
    p = &x;
    if (p != &y) {
      found = 1;
    }
  }
  return found;
}
// end::source[]
