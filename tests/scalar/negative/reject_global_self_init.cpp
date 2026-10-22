// expect: rejected cerastes-unsupported_declaration @ L6
// twin: tests/scalar/const_global.c
// Pins: 0017 item 2: a constant read in its own initializer is dynamically initialized
#include <cerastes.h>

const int g = g + 1;

int get()
{
  return g;
}
