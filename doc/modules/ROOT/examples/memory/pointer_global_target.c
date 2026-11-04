// expect: verified
// expect: native-ok
// mode: c23, c++20
// A contract that names the object a pointer global points to: owns(gp) holds the global pointer, owns(*gp) the object behind it.
// tag::source[]
#include <cerastes.h>

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
// end::source[]
