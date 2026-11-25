// expect: error check @ L11
// mode: c23
// c-only: typeof is a C23 keyword
// twin: tests/scalar/sizeof_typeof_array.c
// Pins: 0063 item 14: sizeof(typeof(int[3])) is 12, not 13 (the same program with a check on the wrong value)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error (a check on the wrong value; the positive is the verbatim program of the flip)
#include <cerastes.h>

int typed(void)
{
  CERASTES_CHECK(sizeof(typeof(int[3])) == 13);  // <- here
  return (int)sizeof(typeof(int[3]));
}
