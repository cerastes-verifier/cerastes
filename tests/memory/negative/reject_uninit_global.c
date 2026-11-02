// expect: rejected attr_requires_automatic_storage @ L7
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 2 (P54, P64): uninit on a global is EDG's own refusal (#1862), re-expected by no session
#include <cerastes.h>

CERASTES_UNINIT int g;  // <- here

int use_global(void)
{
  g = 1;
  return g;
}
