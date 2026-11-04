// expect: rejected attr_redeclaration_mismatch @ L9
// mode: c23, c++20
// twin: tests/vlibc/abort_shim.c
// Pins: 0046 item 6: a different contract after vlibc's header is EDG's must-match error first (0006 item 6), not the reserved-name rejection
// vlibc says `ensures(false)`; this declaration says `ensures(true)`.
#include <stdlib.h>
#include <cerastes.h>

void exit(int status) CERASTES_ENSURES(true);

int f(int x)
    CERASTES_REQUIRES(0 <= x && x <= 10)
{
  return x;
}
