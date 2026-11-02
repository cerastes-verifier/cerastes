// expect: verified
// expect: native-ok
// mode: c23, c++20
// The marker on a local without an initializer states the intent; every read is still checked path by path.
// tag::source[]
#include <cerastes.h>

int pick(int c)
    CERASTES_ENSURES(r, r == 1 || r == 2)
{
  CERASTES_UNINIT int x;
  if (c != 0) {
    x = 1;
  } else {
    x = 2;
  }
  return x;
}
// end::source[]
