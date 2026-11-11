// expect: error uninitialized @ L13
// mode: c23, c++20
// verify-options: --strict-init
// twin: tests/memory/array_uninit_after_name.c
// Pins: 0059 item 11 and 0019: a[2] of the array marked after its name was never written
// Pre-registered from record 0059 item 11 (Part E), before the code
#include <cerastes.h>

int third(void)
{
  int a CERASTES_UNINIT [3];
  a[0] = 1;
  return a[2];
}
