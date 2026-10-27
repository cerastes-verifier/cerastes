// expect: rejected cerastes-constant_logical_operand @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R9: `x && K` (clang -Wconstant-logical-operand, a const int in C too)
#include <cerastes.h>

const int K = 4;

int test(int x, int y)
{
  return x && K;
}
