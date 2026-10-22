// expect: rejected cerastes-unsupported_declaration @ L7
// c-only: in C `const int Z;` is a tentative definition; C++ rejects it (error #257)
// twin: tests/scalar/const_global_tentative.c
// Pins: 0017 item 2: a const definition without an initializer and with external linkage may be defined by another unit (a common symbol)
#include <cerastes.h>

const int Z;

int get(void)
{
  return Z;
}
