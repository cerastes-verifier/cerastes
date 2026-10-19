// expect: error conversion @ L15
// mode: c23, c++20
// twin: tests/scalar/conv_arg.c
// Pins: 0010 item 5: the argument conversion is reported at the call
#include <cerastes.h>

short ident(short s)
{
  return s;
}

short pass_through(int x)
    CERASTES_REQUIRES(x >= -32768 && x <= 40000)
{
  return ident(x);
}
