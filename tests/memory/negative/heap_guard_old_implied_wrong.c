// expect: error postcondition @ L17
// mode: c23, c++20
// twin: tests/memory/heap_guard_old_implied.c
// Pins: 0045 item 3: the body adds 2 where the contract promises old(**pp) + 1 on the guarded path: postcondition at the closing brace
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *c)
    CERASTES_REQUIRES(cerastes_reads(*c) && cerastes_owns(*pp) && (*c == 0 || cerastes_owns(**pp)))
    CERASTES_REQUIRES(*c == 0 || **pp < 100)
    CERASTES_ENSURES(cerastes_reads(*c) && cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*c == 0 || cerastes_owns(**pp)))
    CERASTES_ENSURES(*c == 0 || **pp == cerastes_old(**pp) + 1)
{
  if (*c != 0) {
    **pp = **pp + 2;
  }
}  // <- here
