// expect: rejected cerastes-owner_result_unowned @ L12
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// twin: tests/memory/strict_owner_both.c
// Pins: 0058 item 3: under both options --strict-owner still rejects: calloc (accepted by --strict-init) initializes a plain local: owner_result_unowned
// Pre-registered from record 0058 item 3: rejected owner_result_unowned at the call of calloc
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *t = (int *)calloc(1, sizeof(int));  // <- here
  (void)t;
  return 0;
}
