// expect: error overflow @ L10
// mode: c23, c++20
// twin: tests/memory/static_array_unused.c
// Pins: 0063 item 2: the bound of a static array local is read as written (a prologue constant of the unit): 65536u * 65536u wraps (the front end says int[1]); overflow at the bound
// Pre-registered from record 0063 item 2 (Part B), before any run: the one error
#include <cerastes.h>

int f(void)
{
  [[maybe_unused]] static int a[65536u * 65536u + 1];  // <- here
  return 0;
}
