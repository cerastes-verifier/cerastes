// expect: rejected unrecognized_gcc_pragma @ L8
// expect: rejected unrecognized_gcc_pragma @ L13
// mode: c23, c++20
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a #pragma GCC diagnostic is not recognized by EDG; verify mode makes it an error (#1675)
#include <cerastes.h>

#pragma GCC diagnostic push
int f(void)
{
  return 1;
}
#pragma GCC diagnostic pop
