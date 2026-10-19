// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/ensures_entry.c
// Pins: 0006 item 5: r == x means the entry x, so the incremented result fails it
#include <cerastes.h>

int bump(int x)
    CERASTES_REQUIRES(x >= 0 && x < 100)
    CERASTES_ENSURES(r, r == x)
{
  x = x + 1;
  return x;
}
