// expect: rejected cerastes-self_assignment @ L12
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): parentheses written by a macro around written operands do not hide the self-assignment (clang warns: the x tokens are written)
#include <cerastes.h>

#define LP (
#define RP )

int f(int x)
{
  x = LP x RP;
  return x;
}
