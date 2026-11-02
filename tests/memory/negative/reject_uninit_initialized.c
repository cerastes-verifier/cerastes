// expect: rejected cerastes-uninit_initialized @ L9
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 3: uninit on a local with an initializer (never lifted)
#include <cerastes.h>

int f(void)
{
  CERASTES_UNINIT int x = 1;  // <- here
  return x;
}
