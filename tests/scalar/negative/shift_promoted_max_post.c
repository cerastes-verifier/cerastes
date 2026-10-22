// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/shift_promoted.c
// Pins: 0010 items 6 and 8: 127 << 24 is 2130706432 exactly, so the caller cannot promise one less (the last function of the positive)
#include <cerastes.h>

int top_byte(unsigned char c)
    CERASTES_REQUIRES(c <= 127)
    CERASTES_ENSURES(r, r >= 0 && r == c * 16777216)
{
  return c << 24;
}

// The exact result for the largest byte that fits: a weaker ensures or a stronger requires of `top_byte` fails here.
int top_byte_max(void)
    CERASTES_ENSURES(r, r == 2130706431)
{
  return top_byte(127);
}
