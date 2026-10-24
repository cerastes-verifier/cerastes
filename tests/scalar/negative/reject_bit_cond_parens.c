// expect: rejected cerastes-bitwise_condition_parentheses @ L9
// mode: c23, c++20
// twin: tests/scalar/bit_or.c
// Pins: 0023 item 5: a | (b > 0) ? c : d (clang -Wbitwise-conditional-parentheses)
#include <cerastes.h>

int pick(int a, int b, int c, int d)
{
  return a | (b > 0) ? c : d;
}
