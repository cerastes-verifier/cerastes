// expect: error invalid-free @ L23
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p6.c
// Pins: 0058 item 3 ("accepted although wrong"): an owner by name that is already empty: free(v) twice passes p6 (v is an owner by name), and the default mode's invalid-free reports the second free as it does without the option
// Pre-registered from record 0058 item 3: one invalid-free at the second free under --strict-owner (as heap_strict_init_double_free.c under --strict-init)
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
  CERASTES_OWNING int *v = make();
  free(v);
  free(v);  // <- here
  return 0;
}
