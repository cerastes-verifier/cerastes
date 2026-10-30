// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0041 items 2, 3, 5, 6: a pointer global initialized with &h (an address constant): main holds gp == &h and h, so *gp is h's cell; a callee owns gp (the pointer's own cell) and sets it to NULL, h is framed
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void reset(void)
    CERASTES_REQUIRES(cerastes_owns(gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == NULL)
{
  gp = NULL;
}

int main(void)
{
  if (gp) {
    *gp = 1;
  }
  CERASTES_CHECK(h == 1);
  reset();
  CERASTES_CHECK(gp == NULL && h == 1);
  return 0;
}
