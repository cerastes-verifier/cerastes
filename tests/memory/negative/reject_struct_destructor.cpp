// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a user destructor makes the special members non-trivial (M7)
#include <cerastes.h>

struct D {  // <- here
  int x;
  ~D() {}
};

int ident(int a)
{
  return a;
}
