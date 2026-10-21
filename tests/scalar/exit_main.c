// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0015 items 4 and 5: exit() ends the program, so main need not return on that path
// After the branch that exits, s < 3 holds.
#include <stdlib.h>
#include <cerastes.h>

int status(void)
    CERASTES_ENSURES(r, 0 <= r && r <= 3);

int main(void)
{
  int s = status();
  if (s == 3) {
    exit(1);
  }
  CERASTES_CHECK(s < 3);
  return 0;
}
