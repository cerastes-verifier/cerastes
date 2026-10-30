// expect: error overflow @ L13
// mode: c23, c++20
// twin: tests/memory/cell_postfix_value.c
// Pins: 0039 item 2: x++ as a value on a cell at INT_MAX: the operator's overflow check, before the write effect
#include <cerastes.h>
#include <limits.h>

int post_inc_overflow(void)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x = INT_MAX;
  int *p = &x;
  int y = x++;
  return y + *p;
}
