// expect: golden
// expect: native-ok
// Pins: two invariants on one loop (both on the statement, in order) and an
// invariant on an inner loop inside an outer one (each loop keeps its own).
#include <cerastes.h>

int grid(int n) CERASTES_REQUIRES(n >= 0)
{
  int i = 0;
  int j = 0;
  int c = 0;
  CERASTES_INVARIANT(0 <= i && i <= n) CERASTES_INVARIANT(c >= 0) while (i < n) {
    j = 0;
    CERASTES_INVARIANT(0 <= j && j <= n) while (j < n) {
      c += 1;
      j += 1;
    }
    i += 1;
  }
  return c;
}
