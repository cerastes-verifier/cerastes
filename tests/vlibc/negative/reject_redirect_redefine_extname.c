// expect: rejected cerastes-unsupported_declaration @ L16
// mode: c23, c++20
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 8 (audit B1): `#pragma redefine_extname` gives exit another external name (asm_name)
// The rejection is expected at the call of exit (the pragma is not a declaration).
#include <stdlib.h>
#include <cerastes.h>

void my_exit(int status) { (void) status; }
#pragma redefine_extname exit my_exit

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
