// expect: rejected cerastes-dangling_else @ L15
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R12: through the body of a while loop (written without braces), which carries its annotations
#include <cerastes.h>

int test(int a, int b, int c)
{
  if (a)
    CERASTES_INVARIANT(b >= -1)
    CERASTES_DECREASES(b + 1)
    while (b > 0)
      if (c)
        b--;
      else
        b -= 2;
  return b;
}
