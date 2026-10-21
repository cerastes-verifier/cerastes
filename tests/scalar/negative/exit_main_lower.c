// expect: error check @ L19
// mode: c23, c++20
// twin: tests/scalar/exit_main.c
// Pins: 0015 items 4 and 5: the lower-bound check of the positive needs the lower bound of status()
// status() promises only r <= 3, so a negative s survives the exit branch.
#include <stdlib.h>
#include <cerastes.h>

int status(void)
    CERASTES_ENSURES(r, r <= 3);

int main(void)
{
  int s = status();
  if (s == 3) {
    exit(1);
  }
  CERASTES_CHECK(s < 3);
  CERASTES_CHECK(s >= 0);
  return 0;
}
