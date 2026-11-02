// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a union is rejected (effective types: M8)
#include <cerastes.h>

union U {  // <- here
  int a;
  int b;
};

int ident(int a)
{
  return a;
}
