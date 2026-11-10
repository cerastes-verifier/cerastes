// expect: error leak @ L45
// mode: c23, c++20
// twin: tests/memory/owning_field_return_leaf.c
// Pins: 0055 item 5: the function returns NULL instead, so the leaf still holds its block at the return
// Pre-registered from audit round 2 on Parts B/C (probes r3/): record 0055 item 5: leak at the return statement
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

void eat(void)
    CERASTES_REQUIRES(cerastes_owns(g))
{
  free(g.data);
}

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

CERASTES_OWNING int *f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
{
  eat();
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  if (s.data != NULL) {
    *s.data = 1;
  }
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    i = tick(i);
  }
  return NULL;  // <- here
}
