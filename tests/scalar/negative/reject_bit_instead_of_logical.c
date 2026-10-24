// expect: rejected cerastes-bitwise_instead_of_logical @ L15
// mode: c23, c++20
// twin: tests/scalar/bit_and.c
// Pins: 0023 item 5: & of two bool calls (clang -Wbitwise-instead-of-logical)
#include <cerastes.h>
#include <stdbool.h>

bool pos(int a)
{
  return a > 0;
}

bool both(int a, int b)
{
  return pos(a) & pos(b);
}
