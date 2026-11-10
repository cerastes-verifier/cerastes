// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 5 (limit 4): main fills a struct global's owning leaf and ends holding it after a loop that passes the address of a plain pointer local to a borrower: the loop carries the global's leaf, which it reaches only by type, under the entry snapshot hs of main's holding it
// Pre-registered from record 0056 item 5 and S41 (c) (not yet run on the L-2 build): verifies in both modes; it is the former pin owning_field_loop_main_holds_false_alarm.c (leak at main's closing brace on the base) with a reworded header
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
