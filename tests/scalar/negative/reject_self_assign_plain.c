// expect: rejected cerastes-self_assignment @ L11
// expect: rejected cerastes-self_assignment @ L17
// expect: rejected cerastes-self_assignment @ L23
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): x = x;, x = (x); and (x) = (x); are rejected at the assignment (parentheses do not matter)
#include <cerastes.h>

int plain(int x)
{
  x = x;
  return x;
}

int right_parens(int x)
{
  x = (x);
  return x;
}

int both_parens(int x)
{
  (x) = (x);
  return x;
}
