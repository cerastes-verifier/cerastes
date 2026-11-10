// expect: error leak @ L14
// mode: c23, c++20
// twin: tests/memory/owning_field_value_param_guarded.c
// Pins: 0055 item 6: the callee does not free the block the guarded site gave it: "owning field `s.data` still holds its block when `s`'s lifetime ends", at the return
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: leak at the return statement
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

int take(struct buf s, int k)
    CERASTES_REQUIRES(k != 0 ? (s.data == NULL || cerastes_owns_block(s.data)) : 1)
{
  int n = s.n;
  return n;  // <- here
}
