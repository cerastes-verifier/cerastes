// expect: verified
// expect: native-ok
// mode: c23, c++20
// vlibc's contract for exit is that it does not return, so the path that calls it ends and the postcondition holds on the others.
// tag::source[]
#include <stdlib.h>
#include <cerastes.h>

int non_negative(int x)
    CERASTES_REQUIRES(-100 <= x && x <= 100)
    CERASTES_ENSURES(r, r >= 0 && r == x)
{
  if (x < 0) {
    exit(1);
  }
  return x;
}
// end::source[]
