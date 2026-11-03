// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69 c_inv_pos): *gp of a pointer global in a loop invariant of main (every global is a place of the loop): accepted
#include <cerastes.h>

static int h = 0;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && gp == &h && *gp == i)
  CERASTES_DECREASES(10 - i)
  while (i < 10) {
    h += 1;
    i += 1;
  }
  return gp == &h ? 0 : 1;
}
