// expect: rejected cerastes-owner_result_unowned @ L21
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 (p4): return of a call that gives its result's block from a function that is not owning (its result is a plain pointer): owner_result_unowned at the call
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

int *relay(void)
{
  return make();  // <- here
}
