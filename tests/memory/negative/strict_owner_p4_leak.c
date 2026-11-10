// expect: error leak @ L24
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 (p4): the option only rejects more: an owning local initialized from an owning call and never freed is accepted by p4 and is the default mode's leak, reported at the return
// Pre-registered from record 0058 item 3: one leak at the return of f under --strict-owner
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
  CERASTES_OWNING int *v = make();
  (void)v;
  return 0;  // <- here
}
