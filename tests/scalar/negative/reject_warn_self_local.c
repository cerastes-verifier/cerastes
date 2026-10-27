// expect: rejected cerastes-self_comparison @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: the same local on both sides
#include <cerastes.h>
#include <stdbool.h>

int test(int x)
{
  int l = x;
  return l == l;
}
