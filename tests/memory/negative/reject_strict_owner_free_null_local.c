// expect: rejected cerastes-owner_release_unowned @ L14
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3, audit S7 ("rejected although harmless"): free of a plain local that holds NULL (a bare free(NULL) is unsupported_expression, 0047) releases no block, and p6 still asks for an owner by name: owner_release_unowned at the free (the default mode verifies the program)
// Pre-registered from record 0058 item 3 (audit round 1, S7): rejected owner_release_unowned at the call of free
#include <cerastes.h>
#include <stdlib.h>

int f(void)
    CERASTES_ENSURES(r, r == 0)
{
  int *z = NULL;
  free(z);  // <- here
  return 0;
}
