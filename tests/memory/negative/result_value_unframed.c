// expect: error permission @ L11
// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/live_result.c
// Pins: 0040 item 4 (was the pin reject_result_value.c, 0037 items 5 and 15): *r read with only live(p) held: no earlier atom frames it, permission at the clause, reported in source terms
#include <cerastes.h>
#include <stddef.h>

int *same(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(p) && *r == 1)
{
  return p;
}
