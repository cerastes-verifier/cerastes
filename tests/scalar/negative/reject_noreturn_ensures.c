// expect: rejected cerastes-ensures_on_noreturn @ L8
// mode: c23, c++20
// twin: tests/scalar/noreturn_declared.c
// Pins: 0015 item 1: an ensures on a function that never returns would never be checked
// requires and decreases are allowed (the twin has a requires); an ensures is not.
#include <cerastes.h>

[[noreturn]] void stop(void) CERASTES_ENSURES(true);

int use(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == x)
{
  return x;
}
