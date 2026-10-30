// expect: rejected cerastes-return_local_address @ L11
// expect: frontend-warning returning_ptr_to_local_variable @ L11
// mode: c23, c++20
// twin: tests/memory/pick_owns.c
// Pins: 0039 item 6 (R14): return (&x); is rejected at the return (return (&x);); the front end's warning #1056 prints beside it where it fires
#include <cerastes.h>

int *f(void)
{
  int x = 1;
  return (&x);
}
