// expect: verified
// expect: native-ok
// mode: c23, c++20
// tag::source[]
#include <cerastes.h>

int total(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 40)
{
  int marks[4] = {10, 0, 10, 10};
  int s = 0;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4 && 0 <= s && s <= 10 * i)
  CERASTES_INVARIANT(marks[1] == 0)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    s += marks[i];
    i += 1;
  }
  return s;
}
// end::source[]
