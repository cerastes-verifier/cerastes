// expect: error check @ L12
// mode: c23, c++20
// twin: tests/scalar/typedef_array_used.c
// Pins: 0063 item 14: sizeof(buf_t) of char[4] is 4, not 5 (the same program with a check on the wrong value)
// Pre-registered from record 0063 item 14 (Part D), before any run: the one error (a check on the wrong value; the positive is the verbatim program of the flip)
#include <cerastes.h>

typedef char buf_t[4];

int used(void)
{
  CERASTES_CHECK(sizeof(buf_t) == 5);  // <- here
  return (int)sizeof(buf_t);
}
