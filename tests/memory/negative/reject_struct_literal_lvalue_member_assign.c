// expect: rejected cerastes-unsupported_expression @ L14
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit S2, M8): a compound literal used as an lvalue without &: (struct S){1, 2}.x = 3;
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void f(void)
{
  (struct S){1, 2}.x = 3;  // <- here
}
