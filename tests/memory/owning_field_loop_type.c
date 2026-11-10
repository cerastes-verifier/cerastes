// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 7 (re-audit B2): a function that holds a struct global under owns(g) and runs a loop that passes the address of a plain pointer local to a borrower and stores an int, so the global's leaves keep their flags across the loop (the il-analyst's type_loop.c)
// Pre-registered from the final re-audit (probes r4/ and r5/), observed on the session build after the fixes: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf g;

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

void f(void)
    CERASTES_REQUIRES(cerastes_owns(g))
    CERASTES_ENSURES(cerastes_owns(g))
{
  int *z = NULL;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    bump(&z);
    g.n = i;
    i = tick(i);
  }
}
