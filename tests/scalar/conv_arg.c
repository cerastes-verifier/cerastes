// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0010 item 5 and 0012 item 6: an argument converts to its parameter type, checked at the call
#include <cerastes.h>

short ident(short s)
{
  return s;
}

short pass_through(int x)
    CERASTES_REQUIRES(x >= -32768 && x <= 32767)
{
  return ident(x);
}
