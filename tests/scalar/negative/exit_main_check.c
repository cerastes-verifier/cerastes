// expect: error check @ L18
// mode: c23, c++20
// twin: tests/scalar/exit_main.c
// Pins: 0015 items 4 and 5: only the path that exits is cut off; s == 2 reaches the check
// s < 2 would need the exit to cover 2 as well.
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
  CERASTES_CHECK(s < 2);
  return 0;
}
