// expect: error check @ L10
// mode: c23, c++20
// twin: tests/scalar/alignof_array.c
// Pins: 0063 item 14: alignof(char[4]) is 1, not 4 (the same program with a check on the wrong value)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error (a check on the wrong value; the positive is the verbatim program of the flip)
#include <cerastes.h>

int align(void)
{
  CERASTES_CHECK(alignof(char[4]) == 4);  // <- here
  return (int)alignof(char[4]);
}
