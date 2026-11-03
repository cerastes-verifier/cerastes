// expect: rejected cerastes-unsupported_expression @ L11
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (M8): a literal object in a for step
#include <cerastes.h>

int f(void)
{
  int i = 0;
  CERASTES_DECREASES(3 - i)
  for (; i < 3; i = *&(int){i + 1}) {  // <- here
  }
  return i;
}
