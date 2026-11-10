// expect: error leak @ L53
// mode: c23, c++20
// twin: tests/memory/owning_field_exemption_loop.c
// Pins: 0055 item 5: a plain allocation main still holds at its end is not a static owner's block: the end-of-function row at the closing line
// Pre-registered from audit round 2 on Parts B/C (probes r3/): record 0055 items 4, 5: leak at main's closing brace
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
  int *w = (int *)malloc(sizeof(int));
  if (w != NULL) {
    *w = 1;
  }
  return 0;
}  // <- here
