// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3 (flip of global_pointer_global_deref_atom.c): owns(*gp) names the object the pointer global designates
#include <cerastes.h>
#include <stddef.h>

static int h = 7;
static int *gp = &h;

void f(void)
    CERASTES_REQUIRES(cerastes_owns(gp) && cerastes_owns(*gp))
    CERASTES_ENSURES(cerastes_owns(gp) && gp == cerastes_old(gp) && cerastes_owns(*gp))
{
}

int main(void)
{
  f();
  return gp == &h ? 0 : 1;
}
