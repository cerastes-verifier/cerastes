// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 7 (audit B1): a loop that touches no leaf of a struct local that has an owning field (a leaf ghost the loop does not carry is framed)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): plain_loop_noglobal.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int f(void)
{
  struct buf s = { NULL, 0 };
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = tick(i);
  }
  return s.n;
}
