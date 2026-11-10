// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0055 item 6: a by-value parameter whose site is guarded, k != 0 ? (s.data == NULL || owns_block(s.data)) : 1: the copy's leaf is held at entry only where the guard holds, and the callee frees it under the same guard
// Pre-registered from the il-analyst's item 5 and 6 probes (il/p), the coordinator's task of the audit round: verifies in both modes
#include <cerastes.h>
#include <stdlib.h>
struct buf { int *data CERASTES_OWNING; int n; };

int take(struct buf s, int k)
    CERASTES_REQUIRES(k != 0 ? (s.data == NULL || cerastes_owns_block(s.data)) : 1)
{
  int n = s.n;
  if (k != 0) {
    free(s.data);
  }
  return n;
}
