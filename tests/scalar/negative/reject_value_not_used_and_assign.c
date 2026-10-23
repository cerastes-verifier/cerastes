// expect: rejected cerastes-value_not_used @ L10
// mode: c23, c++20
// twin: tests/scalar/discarded_guarded_effects.c
// Pins: 0020 item 5: the right operand of a discarded && must be an assignment of an integer constant; a variable is warned about by gcc -Wunused-value
#include <cerastes.h>

int f(int a, int b)
{
  int x = 0;
  a && (x = b);
  return x;
}
