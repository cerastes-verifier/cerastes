// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/incdec_uchar.c
// Pins: 0010 item 4: 255 wraps to 0, never to 255
#include <cerastes.h>

unsigned char wrap(unsigned char c)
    CERASTES_REQUIRES(c == 255)
    CERASTES_ENSURES(r, r == 255)
{
  c++;
  return c;
}
