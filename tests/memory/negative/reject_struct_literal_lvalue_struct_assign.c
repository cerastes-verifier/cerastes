// expect: rejected cerastes-unsupported_expression @ L15
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit S2, M8): a compound literal used as an lvalue without &: (struct S){0, 0} = t;
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(void)
{
  struct S t = {1, 2};
  (struct S){0, 0} = t;  // <- here
}
