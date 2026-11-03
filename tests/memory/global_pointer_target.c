// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 3, 4 (question 104): owns(gp) && owns(*gp) for a pointer global: the root is the global's cell, the inner atom reads it plainly; main learns h == 8 through gp == &h
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void bump(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp) && *gp < 100)
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp) && *gp == cerastes_old(*gp) + 1)
{
  *gp += 1;
}

int main(void)
{
  bump();
  CERASTES_CHECK(h == 8);
  return 0;
}
