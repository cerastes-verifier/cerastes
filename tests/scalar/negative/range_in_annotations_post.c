// expect: error postcondition @ L10
// mode: c23, c++20
// twin: tests/scalar/range_in_annotations.c
// Pins: 0025 item 6 R10: the same contract shape fails for a body that returns c, not c + 1
#include <cerastes.h>

unsigned char same(unsigned char c)
    CERASTES_ENSURES(r, r != 300 && r == c + 1)
{
  return c;
}
