// expect: error null-dereference @ L11
// mode: c23, c++20
// twin: tests/memory/array_type_ptr_ptr.c
// Pins: 0059 items 2 and 3: an int ** element left to the zero fill is null
// Pre-registered from the M4.5e1a sweep (report 5, x06)
#include <cerastes.h>

int through(void)
{
  int **pp[2] = {0};
  **pp[1] = 1;
  return 0;
}
