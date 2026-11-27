// expect: error permission @ L19
// mode: c23, c++20
// twin: tests/memory/range_struct_reads.c
// Pins: 0064 item 7 and 0028 item 11 (as range_reads_write.c): a function that only reads p[0..n) over structures writes p[i].x: the write's permission check fails (the amount is below write)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the write
#include <cerastes.h>
#include <stddef.h>

struct P {
  int x;
  int y;
};

void put(struct P *p, int n, int i)
    CERASTES_REQUIRES(cerastes_reads_range(p, n) && 0 <= i && i < n)
    CERASTES_ENSURES(cerastes_reads_range(p, n))
{
  (void)n;
  p[i].x = 1;  // <- here
}
