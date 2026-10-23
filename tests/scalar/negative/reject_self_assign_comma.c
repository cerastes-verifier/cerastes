// expect: rejected cerastes-self_assignment @ L10
// expect: rejected cerastes-self_assignment @ L15
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): a self-assignment as the left operand of a comma, whose value is used or discarded, is rejected
#include <cerastes.h>

int used_comma(int x, int y)
{
  return (x = x, y);
}

int discarded_comma(int x, int y)
{
  x = x, y++;
  return x + y;
}
