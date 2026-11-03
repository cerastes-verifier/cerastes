// expect: rejected cerastes-unsupported_expression @ L11
// c-only: restrict and compound literal objects do not exist in C++
// twin: tests/memory/struct_literal_object.c
// Pins: 0044 item 7 (M8): a literal object in a loop condition is evaluated again without re-entering its block
#include <cerastes.h>

int f(void)
{
  int i = 0;
  CERASTES_DECREASES(3 - i)
  while (*&(int){i} < 3) {  // <- here
    i += 1;
  }
  return i;
}
