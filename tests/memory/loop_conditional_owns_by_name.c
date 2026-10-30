// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0038 item 3 (re-audit): a c == 0 || owns(*p) parameter beside a cell written by name in a loop (x = x + 1), and beside an &param cell (n = n + 1)
#include <cerastes.h>

int by_name(int *p, int c)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)) && r == 10)
{
  if (c != 0) {
    (void)p;
  }
  int x = 0;
  int *q = &x;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && x == i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    x = x + 1;
    i += 1;
  }
  return *q;
}

int by_param(int *p, int c, int n)
    CERASTES_REQUIRES(c == 0 || cerastes_owns(*p))
    CERASTES_ENSURES(r, (c == 0 || cerastes_owns(*p)) && r == 10)
{
  if (c != 0) {
    (void)p;
  }
  int *q = &n;
  int i = 0;
  n = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && n == i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    n = n + 1;
    i += 1;
  }
  return *q;
}
