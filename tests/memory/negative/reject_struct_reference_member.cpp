// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a reference member is a C++ class feature (M7)
#include <cerastes.h>

struct Rf {
  int &r;  // <- here
};

int ident(int a)
{
  return a;
}
