// expect: rejected cerastes-unsupported_declaration @ L9
// c-only: an anonymous struct member is a C11 feature, nonstandard in C++
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: an anonymous struct member is rejected (M8)
#include <cerastes.h>

struct A {
  int x;
  struct {  // <- here
    int y;
    int z;
  };
};

int ident(int a)
{
  return a;
}
