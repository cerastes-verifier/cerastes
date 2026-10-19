// expect: error postcondition @ L15
// mode: c23, c++20
// twin: tests/scalar/void_post.c
// Pins: 0012 item 9: the end of a void function is where the ensures fails when it falls off
#include <cerastes.h>

void guard(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(x > 5)
{
  if (x > 10) {
    return;
  }
  CERASTES_CHECK(x >= 0);
}
