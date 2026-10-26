// expect: rejected cerastes-self_comparison @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R5: a named constant on both sides
#include <cerastes.h>

const int K = 3;

int test(int x)
{
  return K == K;
}
