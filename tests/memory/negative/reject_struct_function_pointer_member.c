// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a pointer-to-function member is rejected (M4.7)
#include <cerastes.h>

struct F {
  int (*fn)(int);  // <- here
  int d;
};

int ident(int a)
{
  return a;
}
