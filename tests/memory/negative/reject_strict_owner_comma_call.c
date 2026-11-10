// expect: rejected cerastes-owner_result_unowned @ L24
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 ("rejected although harmless"): a call as the right operand of a comma that is the receiver's value, v = (n = 1, make());: not peeled either: owner_result_unowned at the call
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
    CERASTES_ENSURES(r, r == 0)
{
  int n = 0;
  CERASTES_OWNING int *v = NULL;
  v = (n = 1, make());  // <- here
  free(v);
  return n - 1;
}
