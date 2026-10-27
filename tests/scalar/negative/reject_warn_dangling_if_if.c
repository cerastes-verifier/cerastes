// expect: rejected cerastes-dangling_else @ L12
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R12: `if (a) if (b) x = 1; else x = 2;`: the else belongs to the inner if (clang by default, gcc -Wall); reported at the else
#include <cerastes.h>

int test(int a, int b, int c, int x)
{
  if (a)
    if (b)
      x = 1;
    else
      x = 2;
  return x;
}
