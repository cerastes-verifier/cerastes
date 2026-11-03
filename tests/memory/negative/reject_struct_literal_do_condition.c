// expect: rejected cerastes-unsupported_expression @ L13
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (M8): a literal object in a do condition
#include <cerastes.h>

int f(void)
{
  int i = 0;
  CERASTES_DECREASES(3 - i)
  do {
    i += 1;
  } while (*&(int){i} < 3);  // <- here
  return i;
}
