// expect: rejected cerastes-unsupported_declaration @ L10
// c-only: variable length arrays are C
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a variable length array is rejected (C only)
#include <cerastes.h>

int vla(int n)
    CERASTES_REQUIRES(n > 0 && n < 100)
{
  [[maybe_unused]] int values[n];
  return n;
}
