// expect: rejected cerastes-return_local_address @ L12
// expect: frontend-warning returning_ptr_to_local_variable @ L12
// mode: c23, c++20
// twin: tests/memory/array_ptr_decay.c
// Pins: 0059 item 10 (0039's R14): return a; gives back the address of an array local: rejected like return &x; (EDG's own warning #1056 listed)
// Pre-registered from record 0059 item 10 (Part D), before the code
#include <cerastes.h>

int *get(void)
{
  int a[3] = {1, 2, 3};
  return a;
}
