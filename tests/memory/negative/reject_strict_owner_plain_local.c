// expect: rejected cerastes-owner_result_unowned @ L12
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 (p4): the result of malloc initializes a plain local, which is no owner: owner_result_unowned at the call (the local is never freed, so p6 has nothing to say; the default mode has the leak)
// Pre-registered from record 0058 item 3: rejected owner_result_unowned at the call of malloc
#include <cerastes.h>
#include <stdlib.h>

int f(void)
{
  int *t = (int *)malloc(sizeof(int));  // <- here
  (void)t;
  return 0;
}
