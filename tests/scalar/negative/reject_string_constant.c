// expect: rejected cerastes-unsupported_expression @ L10
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a string literal is a constant that is not an integer or a bool
#include <cerastes.h>

int tagged(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
{
  return x + (int)(long)"abc";
}
