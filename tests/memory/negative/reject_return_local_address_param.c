// expect: rejected cerastes-return_local_address @ L10
// expect: frontend-warning returning_ptr_to_local_variable @ L10
// mode: c23, c++20
// twin: tests/memory/pick_owns.c
// Pins: 0039 item 6 (R14): return &param; is rejected at the return (the address of a parameter); the front end's warning #1056 prints beside it where it fires
#include <cerastes.h>

int *f(int a)
{
  return &a;
}
