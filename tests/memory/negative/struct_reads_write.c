// expect: error permission @ L16
// mode: c23, c++20
// twin: tests/memory/struct_reads_param.c
// Pins: 0043 items 5, 9: reads(*p) holds a fraction of every leaf: the write fails its permission check
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void put(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(cerastes_reads(*p))
{
  p->x = 1;  // <- here
}
