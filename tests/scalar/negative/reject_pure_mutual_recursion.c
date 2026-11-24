// expect: rejected cerastes-unsupported_annotation @ L11
// expect: rejected cerastes-unsupported_annotation @ L18
// mode: c23, c++20
// twin: tests/scalar/pure_tri.c
// Pins: 0062 item 10: mutual recursion between pure functions is rejected (M4.5f2), each function of the cycle at its definition (the line is the question-160 candidate)
// Pre-registered from record 0062 (Part B), before the code: rejected
#include <cerastes.h>

CERASTES_PURE int is_odd(int n);

CERASTES_PURE int is_even(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 1 : is_odd(n - 1);
}

CERASTES_PURE int is_odd(int n)
    CERASTES_REQUIRES(0 <= n)
    CERASTES_DECREASES(n)
{
  return n == 0 ? 0 : is_even(n - 1);
}
