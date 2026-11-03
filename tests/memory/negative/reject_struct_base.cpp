// expect: rejected cerastes-unsupported_declaration @ L12
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a base class is a C++ class feature (M7)
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct Bs : S {  // <- here
  int z;
};

int ident(int a)
{
  return a;
}
