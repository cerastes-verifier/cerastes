// expect: error uninitialized @ L41
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_site_struct_store.c
// Pins: 0056 item 2, audit round 1 S4: the route of loop_may_fill that is a store of a structure through memory (`*ppr = w;`, a Store of a struct value): the loop has it, so wef is false (a loop that may fill the site gives no guarantee about its flag), and the block of s.data, malloc'd and never written, is returned after the loop: uninitialized at the return. A scan that drops this route makes wef true and moves the error to the loop's entry row, on the line of the loop
// Pre-registered from record 0056 item 2 and audit S4 (designed outcome; build/m4.5d2b2c/t5/r2.c on the L-1 build before filing): uninitialized at the return of s.data, in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct pair {
  int a;
  int b;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  struct pair pr = { 0, 0 };
  struct pair *ppr = &pr;
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    struct pair w = { i, i };
    *ppr = w;
    i = tick(i);
  }
  return s.data;
}
