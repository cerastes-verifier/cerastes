// expect: rejected cerastes-unsequenced_modification @ L10
// mode: c23, c++20
// twin: tests/memory/cell_postfix_value.c
// Pins: 0039 item 3 (audit S9, owner; audit round 2): *&x = x++ names x twice with no sequence point between: rejected like GCC, in both modes (C++17 sequences the right operand first, but one rule serves both modes, as 0020 item 4 has it)
#include <cerastes.h>

int deref_address(void)
{
  int x = 1;
  *&x = x++;
  return x;
}
