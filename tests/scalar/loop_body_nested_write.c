// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0019 item 13 and 0020: a nested write in a loop body sets the flag at its point in the evaluation order
#include <cerastes.h>

int inner(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int i = 0;
  int count = 0;
  CERASTES_INVARIANT(i <= n && count == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int last;
    i = (last = i) + 1;
    count = last + 1;
  }
  return count;
}

int outer(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_ENSURES(r, r == n)
{
  int last;
  int i = 0;
  int count = 0;
  CERASTES_INVARIANT(i <= n && count == i)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    i = (last = i) + 1;
    count = last + 1;
  }
  return count;
}
