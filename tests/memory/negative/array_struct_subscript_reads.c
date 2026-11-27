// expect: error permission @ L17
// mode: c23, c++20
// twin: tests/memory/array_struct_subscript.c
// Pins: 0064 item 5: p[i].x = 1 under cerastes_reads(*p) writes a cell held at a read amount: permission at the write (the null, liveness and bound rows hold: i == 0)
// Pre-registered from record 0064 item 5 (Part B), before any run: the one error
#include <cerastes.h>

struct P {
  int x;
  int y;
};

void set(struct P *p, int i)
    CERASTES_REQUIRES(cerastes_reads(*p) && i == 0)
    CERASTES_ENSURES(cerastes_reads(*p))
{
  p[i].x = 1;  // <- here
}
