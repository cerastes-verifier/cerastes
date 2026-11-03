// expect: rejected cerastes-unsupported_expression @ L30
// c-only: the compound literal spelling of the value is the C shape; the C++ shape is T{{1, 2}, 3} (decision 0044 item 6)
// twin: tests/memory/struct_value_result_use.c
// Pins: 0044 item 5 (audit S4; M8): the destination *pick(p) has a call in its address
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

struct T *pick(struct T *p)
{
  return p;
}

void f(struct T *p)
{
  *pick(p) = (struct T){{1, 2}, 3};  // <- here
}
