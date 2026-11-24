// expect: rejected cerastes-constant_logical_operand @ L11
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3 and 0025 item 6 R9: the right operand of && is an integer constant that is not a truth value, in the body of a read pure function: constant_logical_operand
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the ordinary form, measured on build/m4.5e1d/cerastes-base, is constant_logical_operand at the operator's line)
#include <cerastes.h>

CERASTES_PURE bool both(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return x && 4;
}

int use(int y)
{
  return y;
}
