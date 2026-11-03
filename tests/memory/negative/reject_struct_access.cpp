// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a class with a private member (an access specifier) is a C++ class feature (M7)
#include <cerastes.h>

class Pv {  // <- here
  int x;

 public:
  int y;
};

int ident(int a)
{
  return a;
}
