// expect: rejected cerastes-unsupported_statement @ L13
// mode: c23, c++20
// twin: tests/scalar/switch_default_middle.c
// Pins: 0014 item 1: a declaration that a later case label jumps over is rejected (C)
// C++ only warns here (#546, suppressed in verify mode; an error only for a class with a constructor): cerastes's own rejection is the guard.
#include <cerastes.h>

int skip(int x)
{
  switch (x) {
  case 0:;
    int y = 1;
  case 1:
    return x;
  default:
    return y;
  }
}
