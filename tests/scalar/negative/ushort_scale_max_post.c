// expect: error postcondition @ L18
// mode: c23, c++20
// twin: tests/scalar/ushort_scale.c
// Pins: 0010 item 6: 32768 * 65535 is 2147450880 exactly, so the caller cannot promise one less (the last function of the positive)
#include <cerastes.h>

int scale(unsigned short a)
    CERASTES_REQUIRES(a <= 32768)
    CERASTES_ENSURES(r, r >= 0 && r == a * 65535)
{
  return a * 65535;
}

// The exact result for the largest operand that fits: a weaker ensures or a stronger requires of `scale` fails here.
int scale_max(void)
    CERASTES_ENSURES(r, r == 2147450879)
{
  return scale(32768);
}
