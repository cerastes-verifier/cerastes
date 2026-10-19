// expect: error check @ L10
// A program that fails with the check error.
// tag::source[]
#include <cerastes.h>

int step(int x)
    CERASTES_REQUIRES(0 <= x && x < 100)
{
  int y = x + 10;
  CERASTES_CHECK(y < 100);
  return y;
}
// end::source[]
