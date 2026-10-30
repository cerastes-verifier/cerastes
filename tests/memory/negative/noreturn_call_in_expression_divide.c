// expect: error division-by-zero @ L15
// mode: c23, c++20
// twin: tests/memory/noreturn_call_in_expression.c
// Pins: 0039 item 2 (audit B2): *r may be 0: C may evaluate 100 / *r before nr(), which never returns: division-by-zero at the operator
#include <cerastes.h>
#include <stddef.h>

int nr(void)
    CERASTES_ENSURES(r, false);

int divide_beside(int *q, int *r)
    CERASTES_REQUIRES(cerastes_owns(*q) && cerastes_owns(*r))
    CERASTES_ENSURES(res, cerastes_owns(*q) && cerastes_owns(*r))
{
  return (*q = nr()) + 100 / *r;
}
