// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12: a class (struct) is rejected
#include <cerastes.h>

struct point {
  int x;
  int y;
};

int ident(int a)
{
  return a;
}
