// expect: verified
// expect: native-ok
// mode: c23, c++20
// break, continue, goto and return out of a block that holds a cell: the cell's scope ends at each jump.
// tag::source[]
#include <cerastes.h>

int first_special(int n)
    CERASTES_REQUIRES(0 <= n && n <= 100)
    CERASTES_ENSURES(r, 0 <= r && r <= n)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= n)
  CERASTES_DECREASES(n - i)
  while (i < n) {
    int x = i;
    int *p = &x;
    if (*p == 3) {
      i += 1;
      continue;
    }
    if (*p == 50) {
      break;
    }
    if (*p == 60) {
      goto done;
    }
    if (*p == 70) {
      return i;
    }
    i += 1;
  }
done:
  return i;
}
// end::source[]
