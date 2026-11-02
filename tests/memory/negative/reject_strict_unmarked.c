// expect: rejected cerastes-uninit_unmarked @ L10
// mode: c23, c++20
// twin: tests/memory/init_strict_ok.c
// verify-options: --strict-init
// Pins: 0042 item 8: --strict-init asks for the marker on every local without an initializer
#include <cerastes.h>

int f(void)
{
  int x;  // <- here
  x = 1;
  return x;
}
