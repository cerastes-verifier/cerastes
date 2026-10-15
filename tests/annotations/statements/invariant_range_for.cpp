// expect: golden
// expect: native-ok
// Pins: invariant on a range-based for statement over an array (decision 0006
// item 2 lists range-for); the loop variable is not in scope in the attribute.
#include <cerastes.h>

int total(void)
{
  const int a[3] = {1, 2, 3};
  int s = 0;
  CERASTES_INVARIANT(s >= 0) for (int v : a) {
    s += v;
  }
  return s;
}
