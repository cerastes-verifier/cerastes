// expect: rejected cerastes-unsupported_type @ L9
// mode: c23, c++20
// twin: tests/scalar/sizeof_value.c
// Pins: 0024 item 6: nested arrays
#include <cerastes.h>

int grid(void)
{
  return (int)sizeof(int[2][3]);
}
