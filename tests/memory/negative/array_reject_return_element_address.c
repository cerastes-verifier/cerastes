// expect: rejected cerastes-return_local_address @ L12
// expect: frontend-warning returning_ptr_to_local_variable @ L12
// mode: c23, c++20
// twin: tests/memory/array_element_param.c
// Pins: 0059 item 10 (0039's R14): the address of an element of an array local does not outlive the function; EDG's own warning #1056 is listed
// Pre-registered from record 0059 item 10
#include <cerastes.h>

int *second(void)
{
  int a[3] = {1, 2, 3};
  return &a[1];
}
