// expect: error leak @ L60
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_main_unnamed.c
// Pins: 0056 item 5 (limit 4): the same main with another block malloc'd and never freed after the loop: main's exit row reports it at main's closing brace
// Pre-registered from record 0056 item 5 and S41 (c) v3 (not yet run on the L-2 build): leak at main's closing brace, in both modes
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

void bump(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp))
{
  (void)pp;
}

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

void fill(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  free(g.data);
  g.data = (int *)malloc(sizeof(int));
  if (g.data != NULL) {
    *g.data = 1;
  }
}

int main(void)
{
  int *z = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump(&z);
    i = tick(i);
  }
  int *other = (int *)malloc(sizeof(int));
  (void)other;
  return 0;
}
