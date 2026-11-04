// expect: rejected cerastes-unsupported_declaration @ L10
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 8 (audit B1): an asm label after the header binds exit to another symbol, whose body in the unit returns
// The label is written on a redeclaration after <stdlib.h>, as vlibc's is.
#include <stdlib.h>
#include <cerastes.h>

void my_exit(int status) { (void) status; }
void exit(int status) __asm__("my_exit");

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
