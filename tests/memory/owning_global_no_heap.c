// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0054 item 5 and audit round 2 (Parts C and D), probe b1: an owning global in a program with no allocation at all (no token, no block site anywhere): main assigns it NULL and reads it
// Pre-registered from record 0054 item 5 (audit probe b1; exit 3 "undeclared field tok" when written): verifies in both modes
#include <stddef.h>
#include <cerastes.h>

CERASTES_OWNING static int *g = NULL;

int main(void)
{
  g = NULL;
  return g != NULL;
}
