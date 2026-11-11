// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: an array member is rejected (M4.5e1e: array members, record 0059 item 1)
#include <cerastes.h>

struct A {
  int a[2];  // <- here
  int b;
};

int ident(int a)
{
  return a;
}
