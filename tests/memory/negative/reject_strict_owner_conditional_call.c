// expect: rejected cerastes-owner_result_unowned @ L23
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 ("rejected although harmless"): a call under ?: as the receiver's value, v = c ? make() : NULL;: only casts and lvalue adjustments are peeled, so the call is not seen to reach the owner: owner_result_unowned at the call (the default mode verifies the program)
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

int f(int c)
    CERASTES_ENSURES(r, r == 0)
{
  CERASTES_OWNING int *v = NULL;
  v = c ? make() : NULL;  // <- here
  free(v);
  return 0;
}
