// expect: error leak @ L22
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3, audit S7 ("accepted although wrong"): a result with a partnerless owns(*r) whose closure gives r->data's block is not a gives_result_block call, so p4 does not see the call; discarding it is the default mode's leak
// Pre-registered from record 0058 item 3 (audit round 1, S7): one leak under --strict-owner at the closing brace (observed: build/m4.5d2b2d/tests-A runs)
#include <cerastes.h>
#include <stdlib.h>

struct buf {
  int *data CERASTES_OWNING;
  int n;
};

struct buf *get(void)
    CERASTES_ENSURES(r, r == NULL || cerastes_owns(*r));

int f(void)
{
  get();
  return 0;
}  // <- here
