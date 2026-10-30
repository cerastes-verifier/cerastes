// expect: error null-dereference @ L12
// mode: c23, c++20
// twin: tests/memory/store_value_in_expression.c
// Pins: 0039 item 2 (audit B3): p == NULL || owns(*p) lets p be null: the store inside the expression has its null check, at the store
#include <cerastes.h>
#include <stddef.h>

int store_value(int *p)
    CERASTES_REQUIRES(p == NULL || cerastes_owns(*p))
    CERASTES_ENSURES(r, p == NULL || cerastes_owns(*p))
{
  int y = (*p = 1);
  return y;
}
