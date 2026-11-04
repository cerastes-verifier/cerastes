// expect: error postcondition @ L13
// mode: c23, c++20
// twin: tests/memory/owns_inner_const_cell.c
// Pins: 0045 item 13: the value is wrong: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>

void set(int *const *pp)
    CERASTES_REQUIRES(cerastes_reads(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_reads(*pp) && cerastes_owns(**pp) && **pp == 5)
{
  **pp = 4;
}  // <- here
