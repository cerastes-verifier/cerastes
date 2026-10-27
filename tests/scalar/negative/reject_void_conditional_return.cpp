// expect: rejected cerastes-unsupported_statement @ L28
// mode: c++20
// twin: tests/scalar/void_conditional.c
// Pins: 0025 item 3: C++ allows `return e;` of type void, but a void ?: as a returned expression is not a statement position: rejected like any return with a value in a void function (an M8 entry)
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

void test(int c, int x)
    CERASTES_REQUIRES(c ? x > 0 : x < 0)
{
  return c ? pos(x) : neg(x);
}
