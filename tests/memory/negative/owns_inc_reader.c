// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/owns_inc.c
// Pins: 0037 item 8: a caller that holds only a fraction (reads) cannot give a callee the whole cell: permission at the call
#include <cerastes.h>
#include <stddef.h>

int inc(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p);

int reader(int *p)
    CERASTES_REQUIRES(cerastes_reads(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_reads(*p))
{
  return inc(p);
}
