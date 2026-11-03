// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a bit-field member is rejected (M8), at the member
#include <cerastes.h>

struct B {
  int a : 3;  // <- here
  int b;
};

int ident(int a)
{
  return a;
}
