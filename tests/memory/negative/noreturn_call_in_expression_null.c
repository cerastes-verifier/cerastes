// expect: error null-dereference @ L15
// mode: c23, c++20
// twin: tests/memory/noreturn_call_in_expression.c
// Pins: 0039 item 2 (audit B2): q may be null: C may evaluate the store *q = 1 before nr(), which never returns, so its null check is not hidden: null-dereference at the *q store
#include <cerastes.h>
#include <stddef.h>

int nr(void)
    CERASTES_ENSURES(r, false);

int store_beside(int *r, int *q)
    CERASTES_REQUIRES(cerastes_owns(*r) && (q == NULL || cerastes_owns(*q)))
    CERASTES_ENSURES(res, cerastes_owns(*r) && (q == NULL || cerastes_owns(*q)))
{
  return (*r = nr()) + (*q = 1);
}
