// expect: golden
// expect: native-ok
// check on null statements, in blocks and in both branches of an if.
#include <cerastes.h>

// tag::check[]
int pick(int a, int b) CERASTES_REQUIRES(a >= 0 && b >= 0)
{
  CERASTES_CHECK(a + b >= 0);
  int m;
  if (a < b) {
    CERASTES_CHECK(a < b);
    m = a;
  } else {
    CERASTES_CHECK(a >= b);
    m = b;
  }
  return m;
}
// end::check[]
