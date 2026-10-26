// expect: rejected cerastes-dangling_else @ L9
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R12: the whole statement on one line
#include <cerastes.h>

int test(int a, int b, int c, int x)
{
  if (a) if (b) x = 1; else x = 2;
  return x;
}
