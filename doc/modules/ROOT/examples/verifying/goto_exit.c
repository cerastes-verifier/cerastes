// expect: verified
// expect: native-ok
// mode: c23, c++20
// Forward gotos to a common exit; after the label, each way of getting there is proved separately.
// tag::source[]
#include <cerastes.h>

// x - y, or -1 when y is negative or greater than x.
int checked_difference(int x, int y)
    CERASTES_ENSURES(r, ((y < 0 || x < y) && r == -1) || (0 <= y && y <= x && r == x - y))
{
  int result = -1;
  if (y < 0) {
    goto done;
  }
  if (x < y) {
    goto done;
  }
  result = x - y;
done:
  return result;
}
// end::source[]
