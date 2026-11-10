// expect: error invalid-free @ L28
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// twin: tests/memory/strict_owner_both.c
// Pins: 0058 item 3: the error twin of strict_owner_both.c: under both options the default mode's checks stay: a double free of the owner is reported
// Pre-registered from record 0058 item 3: one invalid-free at the second free under --strict-init --strict-owner
#include <cerastes.h>
#include <stdlib.h>

void init(int *p CERASTES_MUST_INIT)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && *p == 0)
{
  *p = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0 || r == -1)
{
  CERASTES_UNINIT int x;
  init(&x);
  CERASTES_OWNING int *v = (int *)calloc(1, sizeof(int));
  if (v == NULL) {
    return -1;
  }
  int y = *v;
  free(v);
  free(v);  // <- here
  return x + y;
}
