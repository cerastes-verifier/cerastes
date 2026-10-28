// expect: verified
// expect: native-ok
// mode: c23, c++20
// The same program, fixed: x is read through p inside the block, where x still exists.
// tag::source[]
#include <cerastes.h>

int read_in_block(int a)
    CERASTES_ENSURES(r, r == a)
{
  int result;
  {
    int x = a;
    int *p = &x;
    result = *p;
  }
  return result;
}
// end::source[]
