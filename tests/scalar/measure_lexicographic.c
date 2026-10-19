// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0013 item 2: a lexicographic measure; when the first component decreases the second may be anything (negative here)
#include <cerastes.h>

int lex(int m, int n)
    CERASTES_REQUIRES(m >= 0)
    CERASTES_DECREASES(m, n)
    CERASTES_ENSURES(r, r == 0)
{
  if (m == 0) {
    return 0;
  }
  if (n > 0) {
    return lex(m, n - 1);
  }
  return lex(m - 1, -5);
}
