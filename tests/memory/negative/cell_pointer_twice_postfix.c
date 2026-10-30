// expect: rejected cerastes-unsequenced_modification @ L11
// mode: c23, c++20
// twin: tests/memory/cell_pointer_store_postfix.c
// Pins: 0039 item 3 (static): (*p)++ twice with no sequence point between modifies one object through one pointer variable: rejected like the compilers (P61 A)
#include <cerastes.h>

int twice_postfix(void)
{
  int x = 1;
  int *p = &x;
  return (*p)++ + (*p)++;
}
