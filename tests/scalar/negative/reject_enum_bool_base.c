// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c23, c++20
// twin: tests/scalar/enum_fixed.c
// Pins: 0018 item 2: an enumeration with a bool underlying type is rejected at the declaration (C23 and C++ both parse it)
#include <cerastes.h>

enum EB : bool { F, T };

int ident(int a)
{
  return a;
}
