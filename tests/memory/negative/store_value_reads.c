// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/store_value_in_expression.c
// Pins: 0039 item 2 (audit B3): only reads(*q): the store inside the expression needs the whole permission: permission at the store
#include <cerastes.h>
#include <stddef.h>

int store_value(int *q)
    CERASTES_REQUIRES(cerastes_reads(*q))
    CERASTES_ENSURES(r, cerastes_reads(*q))
{
  int y = (*q = 1);
  return y;
}
