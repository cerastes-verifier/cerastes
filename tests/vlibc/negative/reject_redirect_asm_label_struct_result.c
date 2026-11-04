// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/vlibc/user_trusted_declaration.c
// Pins: 0046 item 8 (audit round 2, B2): the same for a routine with a struct result (a tuple of its leaves, 0044), also created early by the run loop
// mk's calls would reach another symbol, whose body returns a struct.
#include <cerastes.h>

struct S { int x; };
struct S mk(void) CERASTES_ENSURES(false) __asm__("h");

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  (void) mk();
  return 0;
}
