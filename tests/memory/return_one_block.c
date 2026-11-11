// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0058 item 2 (audit round 3, the il-analyst's report 8): a function that returns its block parameter p as an owned result: requires owns_block(p) and an ensures that gives the result's block, nothing else: one site names the block, the return verifies
// Pre-registered from audit round 3 (build/m4.5d2b2d/il-analyst-report-8.md): verifies in both modes
#include <cerastes.h>

int *give(int *p)
    CERASTES_REQUIRES(cerastes_owns_block(p))
    CERASTES_ENSURES(r, cerastes_owns_block(r))
{
  return p;
}
