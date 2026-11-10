// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 items 5, 7 (audit B1): main with a struct global of a chain (ga.b->x), consumed by eat() and then a loop that touches nothing: the exemption framing survives the loop
// Pre-registered from audit round 2 on Parts B/C (probes r3/): exm_loop.c: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct leaf {
  int *x CERASTES_OWNING;
  int k;
};

struct mid {
  struct leaf *b CERASTES_OWNING;
  int m;
};

struct mid ga;

void eat(void)
    CERASTES_REQUIRES(cerastes_owns(ga))
{
  if (ga.b != NULL) {
    free(ga.b->x);
    free(ga.b);
  }
}

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

int main(void)
{
  ga.b = NULL;
  ga.m = 0;
  eat();
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = tick(i);
  }
  return 0;
}
