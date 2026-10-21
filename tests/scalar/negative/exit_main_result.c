// expect: error postcondition @ L21
// mode: c23, c++20
// twin: tests/scalar/exit_main.c
// Pins: 0012 item 9 and 0015 items 4 and 5: the ensures of main is checked at its return
// main returns 1 where its ensures says 0 (the exit branch is not the one that returns).
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
  return 1;
}
