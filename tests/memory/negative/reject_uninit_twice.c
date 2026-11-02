// expect: rejected cerastes-unsupported_annotation @ L9
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 3: uninit twice on one declaration (never lifted)
#include <cerastes.h>

int f(void)
{
  CERASTES_UNINIT CERASTES_UNINIT int x;  // <- here
  x = 1;
  return x;
}
