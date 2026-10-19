// expect: verified
// expect: native-ok
// The same program, fixed.
// tag::source[]
#include <cerastes.h>

int step(int x)
    CERASTES_REQUIRES(0 <= x && x < 90)
{
  int y = x + 10;
  CERASTES_CHECK(y < 100);
  return y;
}
// end::source[]
