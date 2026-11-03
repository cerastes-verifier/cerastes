// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0044 item 12 (P69 c_dec_pos): *gp in the measure of a loop of main: accepted
#include <cerastes.h>

static int h = 10;
static int *gp = &h;

int main(void)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 10 && gp == &h && h == 10)
  CERASTES_DECREASES(*gp - i)
  while (i < 10) {
    
    i += 1;
  }
  return gp == &h ? 0 : 1;
}
