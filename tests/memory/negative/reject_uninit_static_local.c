// expect: rejected attr_requires_automatic_storage @ L9
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 2 (P54, P64): uninit on a static local is EDG's own refusal (#1862)
#include <cerastes.h>

int f(void)
{
  CERASTES_UNINIT static int s;  // <- here
  s = 1;
  return s;
}
