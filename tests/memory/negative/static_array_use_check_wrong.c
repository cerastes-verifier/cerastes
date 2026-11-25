// expect: error check @ L12
// mode: c23, c++20
// twin: tests/memory/static_array_use.c
// Pins: 0063 item 9: the same program with a check that is not a fact: t[1] == 1 (the zero rest, or an unknown entry value, is never 1)
// Pre-registered from record 0063 item 9 (Part B), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

int f(void)
{
  static int t[2] = {5};
  CERASTES_CHECK(t[1] == 1);  // <- here
  return t[0];
}
