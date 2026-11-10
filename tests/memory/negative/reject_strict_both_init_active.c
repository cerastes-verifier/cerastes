// expect: rejected cerastes-uninit_unmarked @ L12
// mode: c23, c++20
// verify-options: --strict-init --strict-owner
// twin: tests/memory/strict_owner_both.c
// Pins: 0058 item 3: under both options --strict-init still rejects: a local without an initializer that is not marked (tag as reject_strict_unmarked.c)
// Pre-registered from record 0058 item 3: rejected uninit_unmarked at the declaration
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int x;  // <- here
  x = 1;
  return x;
}
