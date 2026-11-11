// expect: error uninitialized @ L13
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/array_uninit.c
// Pins: 0059 item 11 and 0019: the marker changes nothing about the flags: a[1] was never written
// Pre-registered from record 0059 item 11 (Part E), before the code
#include <cerastes.h>

int second(void)
{
  CERASTES_UNINIT int a[3];
  a[0] = 1;
  return a[1];
}
