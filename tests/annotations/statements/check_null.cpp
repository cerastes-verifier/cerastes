// expect: golden
// expect: native-ok
// Pins: check on a null statement at the start, in a nested block, in the two
// branches of an if, and after a loop; each is a stmk_empty node.
#include <cerastes.h>

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
  {
    CERASTES_CHECK(m <= a && m <= b);
  }
  return m;
}
