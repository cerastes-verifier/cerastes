// expect: rejected cerastes-unsequenced_modification @ L12
// mode: c23, c++20
// twin: tests/memory/cell_postfix_value.c
// Pins: 0039 item 3 (static): x++ + x on a cell modifies and reads the object x with no sequence point between: rejected like the compilers (P61 A)
#include <cerastes.h>

int postfix_plus_name(void)
{
  int x = 1;
  int *p = &x;
  *p = 2;
  return x++ + x;
}
