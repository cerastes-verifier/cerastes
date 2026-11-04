// expect: rejected cerastes-unsupported_declaration @ L9
// c-only: before <stdlib.h> a C++ declaration needs extern "C" and glibc's exception specification would conflict with a written one; the order is the pin
// twin: tests/vlibc/exit_explicit.c
// Pins: 0046 item 8 (audit B1, asm_first.c): the label written BEFORE the header, so the platform's and vlibc's declarations follow it
// The label is part of the routine whichever declaration carries it.
#include <cerastes.h>

void my_exit(int status) { (void) status; }
void exit(int status) __asm__("my_exit");
#include <stdlib.h>

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
    CERASTES_ENSURES(r, r == 1)
{
  exit(x);
  return 0;
}
