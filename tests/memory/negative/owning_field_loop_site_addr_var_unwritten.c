// expect: error uninitialized @ L45
// mode: c23, c++20
// twin: tests/memory/owning_field_loop_site_addr_var.c
// Pins: 0056 item 2, audit round 1 S4: the route of loop_may_fill that is a write of an address-taken variable of the site's pointer type (`z = pk;` with `&z` taken): the loop has it, so wef is false (a loop that may fill the site gives no guarantee about its flag), and the block of s.data, malloc'd and never written, is returned after the loop: uninitialized at the return. A scan that drops this route makes wef true and moves the error to the loop's entry row, on the line of the loop
// Pre-registered from record 0056 item 2 and audit S4 (designed outcome; build/m4.5d2b2c/t5/r3.c on the L-1 build before filing): uninitialized at the return of s.data, in both modes
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

int tick(int i)
    CERASTES_REQUIRES(0 <= i && i < 10)
    CERASTES_ENSURES(r, r == i + 1)
{
  return i + 1;
}

void keep(int **pp)
    CERASTES_REQUIRES(cerastes_owns(*pp))
    CERASTES_ENSURES(cerastes_owns(*pp) && *pp == cerastes_old(*pp))
{
  (void)pp;
}

CERASTES_OWNING int *f(void)
{
  struct buf s;
  s.n = 0;
  s.data = (int *)malloc(sizeof(int));
  int k = 0;
  int *pk = &k;
  int *z = NULL;
  int **pz = &z;
  keep(pz);
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    z = pk;
    i = tick(i);
  }
  return s.data;
}
