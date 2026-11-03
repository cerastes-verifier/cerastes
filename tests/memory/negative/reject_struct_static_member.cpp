// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a static data member is a C++ class feature (M7)
#include <cerastes.h>

struct St {  // <- here
  int x;
  static int c;
};

int ident(int a)
{
  return a;
}
