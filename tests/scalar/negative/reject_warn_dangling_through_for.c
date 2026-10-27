// expect: rejected cerastes-dangling_else @ L17
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R12: through the body of a for loop
#include <cerastes.h>

int test(int a, int b, int c, int x)
    CERASTES_REQUIRES(0 <= b && b < 100)
{
  int i;
  if (a)
    CERASTES_INVARIANT(0 <= i && i <= b)
    CERASTES_DECREASES(b - i)
    for (i = 0; i < b; i++)
      if (c)
        x = 1;
      else
        x = 2;
  return x;
}
