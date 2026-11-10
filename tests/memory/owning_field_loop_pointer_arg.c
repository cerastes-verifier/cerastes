// expect: verified
// expect: native-ok
// mode: c23, c++20
// Covers: 0056 item 2 (limit 5): an owning function returns a struct local's leaf block after a loop that passes the address of a plain pointer local to a borrower and stores no int: the loop cannot fill the leaf's type, so the leaf's contents flag survives it (S41 (0) run 5a: the same fact written by hand as a loop invariant verifies)
// Pre-registered from record 0056 item 2 (not yet run): verifies in both modes; it is the former pin owning_field_loop_pointer_arg_false_alarm.c (uninitialized @ L54 on the base) with a reworded header
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
