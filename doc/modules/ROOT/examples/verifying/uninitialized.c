// expect: verified
// expect: native-ok
// mode: c23, c++20
// A local without an initializer: read only on the paths where it was assigned.
// tag::source[]
#include <cerastes.h>

int pick(int c, int a)
    CERASTES_ENSURES(r, (c != 0 && r == a) || (c == 0 && r == 0))
{
  int x;
  if (c) {
    x = a;
  }
  if (c) {
    return x;
  }
  return 0;
}

int sign(int a)
    CERASTES_ENSURES(r, (a > 0 && r == 1) || (a == 0 && r == 0) || (a < 0 && r == -1))
{
  int s;
  if (a > 0) {
    s = 1;
  } else if (a < 0) {
    s = -1;
  } else {
    s = 0;
  }
  return s;
}
// end::source[]
