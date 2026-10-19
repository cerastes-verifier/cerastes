// expect: error postcondition @ L11
// twin: tests/scalar/old_cpp.cpp
// Pins: 0012 item 2: old(x) is the entry value, so r == old(x) fails
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == cerastes::old(x))
{
  x = x + 1;
  return x;
}
