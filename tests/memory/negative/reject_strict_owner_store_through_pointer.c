// expect: rejected cerastes-owner_result_unowned @ L24
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3, audit S7 ("rejected although harmless"): *pp = make() with pp == &v: the store goes through a pointer, so the receiver is not an owner by name: owner_result_unowned at the call (the default mode verifies the program)
// Pre-registered from record 0058 item 3 (audit round 1, S7): rejected owner_result_unowned at the call of make
#include <cerastes.h>
#include <stdlib.h>

CERASTES_OWNING int *make(void)
{
  CERASTES_OWNING int *a = (int *)malloc(sizeof(int));
  if (a != NULL) {
    *a = 0;
  }
  return a;
}

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = NULL;
  int **pp = &v;
  *pp = make();  // <- here
  free(v);
  return 0;
}
