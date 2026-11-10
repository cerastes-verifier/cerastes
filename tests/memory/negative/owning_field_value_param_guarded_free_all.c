// expect: error invalid-free @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_value_param_guarded.c
// Pins: 0055 item 6: the site is guarded, so where k == 0 the callee holds no block of s.data and the unconditional free is invalid (byval_guard_bad.c)
// Pre-registered from audit round 2 on Parts B/C (probes r3/): byval_guard_bad.c: invalid-free at the free
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

int take(struct buf s, int k)
    CERASTES_REQUIRES(k != 0 ? (s.data == NULL || cerastes_owns_block(s.data)) : 1)
{
  int n = s.n;
  free(s.data);  // <- here
  return n;
}
