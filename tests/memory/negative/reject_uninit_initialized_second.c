// expect: rejected cerastes-uninit_initialized @ L9
// mode: c23, c++20
// twin: tests/memory/init_uninit_local.c
// Pins: 0042 item 3: a leading marker covers every declarator of its declaration, so it reaches b, which has an initializer
#include <cerastes.h>

int f(void)
{
  CERASTES_UNINIT int a, b = 1;  // <- here
  a = 2;
  return a + b;
}
