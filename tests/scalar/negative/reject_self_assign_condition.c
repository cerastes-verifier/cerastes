// expect: rejected cerastes-self_assignment @ L10
// expect: rejected cerastes-self_assignment @ L21
// mode: c23, c++20
// twin: tests/scalar/self_assign_not_warned.c
// Pins: 0021 item 8 (clang -Wself-assign): a self-assignment as the condition of an if or a while (with the extra parentheses) is rejected
#include <cerastes.h>

int in_if(int x)
{
  if ((x = x)) {
    return 1;
  }
  return 0;
}

int in_while(int x)
    CERASTES_REQUIRES(0 <= x)
{
  CERASTES_INVARIANT(0 <= x)
  CERASTES_DECREASES(x)
  while ((x = x)) {
    x = x - 1;
  }
  return x;
}
