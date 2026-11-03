// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a void * member is rejected (0028 item 1)
#include <cerastes.h>

struct P {
  void *p;  // <- here
  int d;
};

int ident(int a)
{
  return a;
}
