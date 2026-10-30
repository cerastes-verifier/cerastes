// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 items 3 and 6: a cell in a loop invariant (the shape that b1a rejected): the clause is a fact about the value the place keeps
#include <cerastes.h>

int invariant_cell(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 0;
  int *p = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && x == 0)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i += 1;
  }
  return *p;
}
