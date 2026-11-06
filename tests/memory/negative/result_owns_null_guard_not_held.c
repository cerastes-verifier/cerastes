// expect: error permission @ L15
// mode: c23, c++20
// twin: tests/memory/result_owns_null_guard.c
// Pins: 0052 item 2 (audit B1): the same guarded atom without holding g: the non-NULL return fails its row, the NULL return does not: permission at the non-NULL return only
// Pre-registered from record 0052 item 2: permission at the non-NULL return
#include <cerastes.h>
#include <stddef.h>

static int g = 3;

int *maybe(int c)
    CERASTES_ENSURES(r, r == NULL || (cerastes_owns(*r) && r == &g))
{
  if (c != 0) {
    return &g;  // <- here
  }
  return NULL;
}
