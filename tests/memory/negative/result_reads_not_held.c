// expect: error permission @ L13
// mode: c23, c++20
// twin: tests/memory/result_reads.c
// Pins: 0052 item 2: reads(*r) of g returned without holding g: permission at the return
// Pre-registered from record 0052 item 2: permission at the return statement
#include <cerastes.h>

static int g = 4;

int *view(void)
    CERASTES_ENSURES(r, cerastes_reads(*r) && r == &g)
{
  return &g;  // <- here
}
