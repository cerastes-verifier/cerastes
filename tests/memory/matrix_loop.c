// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 10 and 5 (Part E; 0059 item 8): nested loops over a local matrix. fill writes m[i][j] in two nested loops over an initialized matrix (each loop carries the permission of every cell by two bound variables, the flags and the type's range, not the values) and reads a cell after them (initialized, its value times 0); sum reads m[i][j] in two nested loops over a matrix whose values are known (the values are kept by a loop that never writes them) and bounds the sum by the largest element
// Pre-registered from record 0064 items 5 and 10 (Part E), before any run: verifies
#include <cerastes.h>

int fill(void)
    CERASTES_ENSURES(r, r == 6)
{
  int m[2][3] = {{0, 0, 0}, {0, 0, 0}};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    int j = 0;
    CERASTES_INVARIANT(0 <= j && j <= 3)
    CERASTES_DECREASES(3 - j)
    while (j < 3) {
      m[i][j] = i + j;
      j = j + 1;
    }
    i = i + 1;
  }
  return i * 3 + m[1][2] * 0;
}

int sum(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 36)
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
  return s;
}
