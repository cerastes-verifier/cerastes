// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2: the int **pp form: owns(*pp) && owns(**pp) in and out, so the place **pp goes through the load *pp and the implicit ensures says *pp == old(*pp); the body writes the int; the caller checks that q still points to x
// Pre-registered from record 0058 item 2: verifies in both modes (the written form, an explicit *pp == old(*pp), verifies today: build/m4.5d2b2d/emul/d1_emul.c)
#include <cerastes.h>

void zero(int **pp CERASTES_NOT_INVALIDATING)
    CERASTES_REQUIRES(cerastes_owns(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && cerastes_owns(**pp))
{
  **pp = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  int x = 5;
  int *q = &x;
  zero(&q);
  CERASTES_CHECK(q == &x);
  return 0;
}
