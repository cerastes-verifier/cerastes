// expect: rejected cerastes-constant_logical_operand @ L11
// mode: c23, c++20
// twin: tests/scalar/warn_accepted_neighbours.c
// Pins: 0025 item 6 R9: `x && ONE9` (clang -Wconstant-logical-operand, an enumerator of value 1: clang C++ only)
#include <cerastes.h>

enum e9 { Z9, ONE9 };

int test(int x, int y)
{
  return x && ONE9;
}
