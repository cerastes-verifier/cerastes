// expect: rejected cerastes-unsupported_declaration @ L7
// mode: c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: an empty struct is rejected
#include <cerastes.h>

struct E {};  // <- here

int ident(int a)
{
  return a;
}
