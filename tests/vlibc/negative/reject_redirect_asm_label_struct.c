// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/vlibc/user_trusted_declaration.c
// Pins: 0046 item 8 (audit round 2, B2): a routine with a struct-pointer parameter is created early by the run loop; its asm label must still be refused at its calls
// g's calls would reach another symbol, whose body returns: before the fix this program verified (the native binary returns).
#include <cerastes.h>

struct S { int x; };
void g(struct S *p) CERASTES_ENSURES(false) __asm__("h");

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  g((struct S *) 0);
  return 0;
}
