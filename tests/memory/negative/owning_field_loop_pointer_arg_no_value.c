// expect: error uninitialized @ L51
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_pointer_arg.c
// Pins: 0056 item 2 (limit 5): the same function without the store *s.data = 1: the returned block holds no value, so the flag the loop keeps is the true one and the return reports it
// Pre-registered from record 0056 item 2 (not yet run): uninitialized at the return of s.data (where the former pin reported it), for a true reason now
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
