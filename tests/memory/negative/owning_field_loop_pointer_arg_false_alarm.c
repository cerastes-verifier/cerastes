// expect: error uninitialized @ L54
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_type.c
// Pins: 0055 item 7 (a false alarm pinned): an owning function returns a struct local's leaf block after a loop that passes the address of a plain pointer local to a borrower and stores no int: a leaf carried by type loses its contents' flag when the loop neither stores that type nor passes the owner, so the returned block is reported as having no value; with an int store in the loop (owning_field_loop_type.c) it verifies. The expectations are the errors the encoder reports today
// Pre-registered from the final re-audit (probes r4/ and r5/), observed on the session build after the fixes: ret_leaf_type.c: the expect lines are the actual outcome, recorded as a pin
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
  int *z = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump(&z);
    i = tick(i);
  }
  return s.data;
}
