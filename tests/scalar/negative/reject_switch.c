// expect: rejected cerastes-unsupported_statement @ L10
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: switch is rejected
// Assumption A1 (INDEX.md): one diagnostic, at the switch.
#include <cerastes.h>

int pick(int x)
{
  switch (x) {
  default:
    return 0;
  }
}
