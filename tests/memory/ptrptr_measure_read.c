// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 12: a loop measure that reads **pp (n - **pp), through the automatic places
#include <cerastes.h>

void count(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
{
  int x = 0;
  int *p = &x;
  int **pp = &p;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n && **pp == i)
  CERASTES_DECREASES(n - **pp)
  while (i < n) {
    **pp += 1;
    i += 1;
  }
}
