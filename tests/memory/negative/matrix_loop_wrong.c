// expect: error postcondition @ L28
// mode: c23, c++20
// twin: tests/memory/matrix_loop.c
// Pins: 0064 item 10: the invariants of the nested sum bound s by 36, no more (the loops' own conjuncts are checked and hold), so the stronger postcondition r <= 35 fails at the return: one error, and none inside the loops
// Pre-registered from record 0064 item 10 (Part E), before any run: the one error
#include <cerastes.h>

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 35)
{
  int m[2][3] = {{1, 2, 3}, {4, 5, 6}};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_INVARIANT(0 <= s && s <= 18 * i)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3)
    CERASTES_INVARIANT(0 <= s && s <= 18 * i + 6 * j)
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      s = s + m[i][j];
      j = j + 1;
    }
    i = i + 1;
  }
  return s;  // <- here
}
