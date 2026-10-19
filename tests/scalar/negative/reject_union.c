// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a union is rejected
#include <cerastes.h>

union either {
  int i;
  short s;
};

int ident(int a)
{
  return a;
}
