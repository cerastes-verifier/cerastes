// expect: rejected cerastes-unsupported_expression @ L12
// mode: c23, c++20
// twin: tests/scalar/pure_check_small.c
// Pins: 0062 item 3: & of a parameter in the body of a read pure function is unsupported_expression (forall_reject_address_of_param.c)
// Pre-registered from record 0062 (Part B), before the code: rejected. the line is a question-160 candidate (the construct's own line: the function is read by its requires alone)
#include <cerastes.h>
#include <stddef.h>

CERASTES_PURE bool g(int x)
    CERASTES_REQUIRES(0 <= x)
{
  return &x != NULL;
}

int use(int y)
{
  return y;
}
