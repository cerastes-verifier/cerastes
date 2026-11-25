// expect: error check @ L10
// mode: c23, c++20
// twin: tests/scalar/sizeof_array_type.c
// Pins: 0063 item 14: sizeof(int[3]) is 12, not 13 (the same program with a check on the wrong value)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error (a check on the wrong value; the positive is the verbatim program of the flip)
#include <cerastes.h>

int three_ints(void)
{
  CERASTES_CHECK(sizeof(int[3]) == 13);  // <- here
  return (int)sizeof(int[3]);
}
