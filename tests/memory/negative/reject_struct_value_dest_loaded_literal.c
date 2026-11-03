// expect: rejected cerastes-unsupported_expression @ L25
// c-only: the compound literal spelling of the value is the C shape; the C++ shape is S{1, 2} (decision 0044 item 6)
// twin: tests/memory/struct_value_result_use.c
// Pins: 0044 item 5 (audit S4; M8): the destination (*pp)->t with a compound literal as the value
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
  (*pp)->t = (struct S){1, 2};  // <- here
}
