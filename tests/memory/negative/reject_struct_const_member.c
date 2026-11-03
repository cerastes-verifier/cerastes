// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a const member is rejected (M4.6)
#include <cerastes.h>

struct C {
  const int c;  // <- here
  int d;
};

int ident(int a)
{
  return a;
}
