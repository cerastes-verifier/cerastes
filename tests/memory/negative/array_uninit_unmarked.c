// expect: rejected cerastes-uninit_unmarked @ L11
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/array_uninit.c
// Pins: 0059 item 11 (0042 item 8): under --strict-init an array local without an initializer needs the marker, as a scalar does
// Pre-registered from record 0059 item 11 (Part E), before the code
#include <cerastes.h>

int first(void)
{
  int a[3];
  a[0] = 1;
  return a[0];
}
