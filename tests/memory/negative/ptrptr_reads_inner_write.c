// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/ptrptr_reads_inner.c
// Pins: 0045 item 5: a write under reads(**pp) needs the whole permission: permission at the write
#include <cerastes.h>
#include <stddef.h>

int peek2(int **pp)
    CERASTES_REQUIRES(cerastes_reads(*pp) && cerastes_reads(**pp) && **pp < 100)
    CERASTES_ENSURES(r, cerastes_reads(*pp) && cerastes_reads(**pp) && r == **pp)
{
  **pp = 0;  // <- here
  return 0;
}
