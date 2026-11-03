// expect: rejected cerastes-unsupported_expression @ L25
// mode: c23, c++20
// twin: tests/memory/struct_value_result_use.c
// Pins: 0044 item 5 (audit S4; M8): the destination (*pp)->t is reached through a loaded pointer, and mk could write *pp: rejected
#include <cerastes.h>

struct S {
  int x;
  int y;
};

struct T {
  struct S t;
  int z;
};

struct S mk(void)
{
  struct S s = {1, 2};
  return s;
}

void f(struct T **pp)
{
  (*pp)->t = mk();  // <- here
}
