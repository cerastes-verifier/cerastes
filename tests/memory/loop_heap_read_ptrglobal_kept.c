// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69 c_trap_dec): the loop never writes gp, so it keeps &h and *gp is read through it unpinned: accepted (as global_loop.c pins for g)
#include <cerastes.h>

static int h = 10;
static int k = 0;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && h == 10 - i && *gp >= 0)
  CERASTES_DECREASES(*gp)
  while (i < 10) {
    h -= 1;
    i += 1;
  }
  return k + (gp == &h ? 0 : 1);
}
