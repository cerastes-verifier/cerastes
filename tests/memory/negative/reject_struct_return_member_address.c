// expect: rejected cerastes-return_local_address @ L16
// expect: frontend-warning returning_ptr_to_local_variable @ L16
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 5 (R14): the address of a member of a local struct escapes: rejected at the return, the front end's #1056 beside it
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int *leak(void)
{
  struct S s = {1, 2};
  return &s.x;  // <- here
}
