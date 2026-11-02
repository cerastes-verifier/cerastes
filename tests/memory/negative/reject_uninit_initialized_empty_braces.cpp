// expect: rejected cerastes-uninit_initialized @ L9
// mode: c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 3: C++'s `int x = {}` is an initializer
#include <cerastes.h>

int f()
{
  CERASTES_UNINIT int x = {};  // <- here
  return x;
}
