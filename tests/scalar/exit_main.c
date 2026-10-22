// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 items 4 and 5: exit() ends the program, so main need not return on that path
// After the branch that exits, s < 3 holds, and so does the lower bound of status().
#include <stdlib.h>
#include <cerastes.h>

int status(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 3);

int main(void)
    CERASTES_ENSURES(r, r == 0)
{
  int s = status();
  if (s == 3) {
    exit(1);
  }
  CERASTES_CHECK(s < 3);
  CERASTES_CHECK(s >= 0);
  return 0;
}
