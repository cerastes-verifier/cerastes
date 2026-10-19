// expect: error conversion @ L10
// mode: c23, c++20
// twin: tests/scalar/incdec_short.c
// Pins: 0010 item 5: 32767++ does not fit short
#include <cerastes.h>

short next(short s)
    CERASTES_ENSURES(r, r == s + 1)
{
  s++;
  return s;
}
