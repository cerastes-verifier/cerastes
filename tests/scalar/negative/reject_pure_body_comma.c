// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: the comma operator in the body of a read pure function is unsupported_expression (forall_reject_e_comma.c)
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the construct's own line: the function is read by its requires alone)
#include <cerastes.h>


CERASTES_PURE bool g(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return ((void)x, x > 0);
}

int use(int y)
{
  return y;
}
