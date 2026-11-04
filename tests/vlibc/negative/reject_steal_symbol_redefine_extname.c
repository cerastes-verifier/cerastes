// expect: rejected cerastes-unsupported_declaration @ L11
// c-only: in C++ EDG requires a function renamed by `#pragma redefine_extname` to have C linkage (error #1199), which a dual-mode file cannot write; the C run is the pin
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 8 (re-audit): `#pragma redefine_extname` renaming a program function to exit's symbol is refused like an asm label
// The rejection is expected at the renamed function's declaration.
#include <stdlib.h>
#include <cerastes.h>

#pragma redefine_extname my_exit exit

void my_exit(int status)
{
  (void) status;
}

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
