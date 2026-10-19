// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a non-const global is rejected
// Assumption A2 (INDEX.md): the diagnostic is at the declaration, not at a use.
#include <cerastes.h>

int counter = 0;

int ident(int a)
{
  return a;
}
