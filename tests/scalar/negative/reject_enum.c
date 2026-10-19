// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/if_else.c
// Pins: 0012 item 12 and 0010 item 2: an enumeration is rejected in both modes
#include <cerastes.h>

enum color {
  RED,
  GREEN
};

int ident(int a)
{
  return a;
}
