// expect: error postcondition @ L12
// mode: c23, c++20
// twin: tests/scalar/void_post.c
// Pins: 0012 item 9: the early return of a void function is where the ensures fails
#include <cerastes.h>

void guard(int x)
    CERASTES_REQUIRES(x >= 0)
    CERASTES_ENSURES(x > 0)
{
  if (x == 0) {
    return;
  }
  CERASTES_CHECK(x > 0);
}
