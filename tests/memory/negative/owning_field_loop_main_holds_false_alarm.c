// expect: error leak @ L61
// mode: c23, c++20
// twin: tests/memory/owning_field_global_main.c
// Pins: 0055 item 7 (a false alarm pinned, not a defect of the program): main fills a struct global's owning leaf and ends holding it after a loop that passes the address of a plain pointer local to a borrower: the loop does not carry the global's leaf, which it reaches only by type, so main's end row reports the block the static owner holds; without the loop (owning_field_global_main.c) it verifies. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (probes r4/ and r5/), observed on the session build after the fixes: main_hold_type.c: the expect lines are the actual outcome, recorded as a pin
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
  g.data = NULL;
  g.n = 0;
  fill();
  int *z = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump(&z);
    i = tick(i);
  }
  return 0;
}
