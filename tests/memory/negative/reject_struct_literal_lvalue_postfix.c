// expect: rejected cerastes-unsupported_expression @ L9
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (audit S2, M8): a compound literal used as an lvalue without &: (int){1}++;
#include <cerastes.h>

void f(void)
{
  (int){1}++;  // <- here
}
