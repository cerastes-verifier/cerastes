// expect: rejected cerastes-unsequenced_modification @ L11
// mode: c23, c++20
// twin: tests/memory/write_through_pointer.c
// Pins: 0039 item 3 (static): *p = (*p)++ modifies and reads *p through one pointer variable with no sequence point between (GCC -Wsequence-point warns, P61 A4; re-expected by M4.5b1c2)
#include <cerastes.h>

int self_assign(void)
{
  int x = 1;
  int *p = &x;
  *p = (*p)++;
  return x;
}
