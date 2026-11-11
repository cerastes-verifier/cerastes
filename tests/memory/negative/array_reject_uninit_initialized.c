// expect: rejected cerastes-uninit_initialized @ L10
// mode: c23, c++20
// twin: tests/memory/array_uninit.c
// Pins: 0059 item 11 (0042 item 3): the marker says the object starts without a value: an array with a braced initializer is rejected, at the declaration
// Pre-registered from record 0059 item 11 (Part E), before the code
#include <cerastes.h>

int first(void)
{
  CERASTES_UNINIT int a[3] = {1, 2, 3};
  return a[0];
}
