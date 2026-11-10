// expect: rejected cerastes-owner_result_unowned @ L18
// mode: c23, c++20
// verify-options: --strict-owner
// twin: tests/memory/strict_owner_p4.c
// Pins: 0058 item 3 (p4): the result of malloc is stored in a field that is not owning (struct plain has no owning field): owner_result_unowned at the call
// Pre-registered from record 0058 item 3: rejected owner_result_unowned at the call of malloc
#include <cerastes.h>
#include <stdlib.h>

struct plain {
  int *q;
  int n;
};

int f(void)
{
  struct plain s = { NULL, 0 };
  s.q = (int *)malloc(sizeof(int));  // <- here
  return s.n;
}
