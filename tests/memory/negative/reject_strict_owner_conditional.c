// expect: rejected cerastes-owner_release_unowned @ L22
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 (p6, "rejected although harmless"): free of a conditional of owners (here both arms name the same owner, so the default mode accepts it): owner_release_unowned at the free
// Pre-registered from record 0058 item 3: rejected owner_release_unowned at the call of free
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
{
  CERASTES_OWNING int *a = make();
  free(c ? a : a);  // <- here
  return 0;
}
