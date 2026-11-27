// expect: verified
// expect: native-ok
// mode: c23, c++20
// A matrix summed by two nested loops: the outer invariant bounds the rows done so far, the inner one the cells of the current row. The loops write nothing, so the matrix keeps its values and its permissions through them.
// Pre-registered from record 0064 items 3, 5 and 6 (Part E, the loops step), before any run: verifies. It cannot run before the loops over a local matrix are lowered.
// tag::source[]
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 36)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2 && 0 <= s && s <= 18 * i)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3 && 0 <= s && s <= 18 * i + 6 * j)
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      s += m[i][j];
      j += 1;
    }
    i += 1;
  }
  return s;
}
// end::source[]
