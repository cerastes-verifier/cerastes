// expect: rejected cerastes-unsupported_declaration @ L9
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a struct defined at block scope is rejected (M8)
#include <cerastes.h>

int block_struct(void)
{
  struct B {  // <- here
    int a;
  } b;
  b.a = 1;
  return b.a;
}
