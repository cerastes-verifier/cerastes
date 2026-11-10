// expect: rejected cerastes-owner_result_unowned @ L21
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 (p4): the result of an owning call is discarded in an expression statement: owner_result_unowned at the call (P4417R0 p4: it must initialize or be assigned to an owner, or be deleted, in its full-expression)
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
  make();  // <- here
  return 0;
}
