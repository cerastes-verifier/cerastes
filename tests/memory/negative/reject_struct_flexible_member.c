// expect: rejected cerastes-unsupported_declaration @ L9
// c-only: a flexible array member is not C++
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a flexible array member is rejected (M4.5e1)
#include <cerastes.h>

struct F {
  int n;
  int a[];  // <- here
};

int ident(int a)
{
  return a;
}
