// expect: rejected cerastes-unsupported_type @ L28
// mode: c23, c++20
// twin: tests/scalar/void_conditional.c
// Pins: 0025 item 3: a void ?: has no value; as the left operand of a comma whose value is used it is not in statement position (the encoder splits that comma's left operand only when the comma's value is discarded), so it stays rejected
#include <cerastes.h>

void pos(int x)
    CERASTES_REQUIRES(x > 0)
{
  (void)x;
}

void neg(int y)
    CERASTES_REQUIRES(y < 0)
{
  (void)y;
}

int id(int a)
    CERASTES_ENSURES(r, r == a)
{
  return a;
}

int test(int c, int x, int y)
    CERASTES_REQUIRES(c ? x > 0 : y < 0)
{
  int z = ((c ? pos(x) : neg(y)), 5);
  return z;
}
