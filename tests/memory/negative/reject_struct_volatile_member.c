// expect: rejected cerastes-unsupported_declaration @ L8
// mode: c23, c++20
// twin: tests/memory/struct_local.c
// Pins: 0043 item 2: a volatile member is rejected (M8)
#include <cerastes.h>

struct V {
  volatile int v;  // <- here
  int d;
};

int ident(int a)
{
  return a;
}
