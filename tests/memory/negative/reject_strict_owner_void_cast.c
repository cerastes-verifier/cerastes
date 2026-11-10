// expect: rejected cerastes-owner_result_unowned @ L21
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 ("rejected although..."): the result discarded by a cast to void, (void)make();: P4417R0 p4 asks for an owner or a delete in the full-expression, so this is rejected (owner_result_unowned at the call); the default mode reports the leak, so here the rejection is not harmless but earlier
// Pre-registered from record 0058 item 3: rejected owner_result_unowned at the call of make
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
{
  (void)make();  // <- here
  return 0;
}
