// expect: error unsequenced @ L30
// mode: c23, c++20
// twin: tests/memory/two_memory_calls.c
// Pins: 0039 item 4: bump(p) + peek(q) with q = c ? p : &y: the owns atom and the reads atom may name one cell on the path c != 0, so arg1 != arg2 fails
#include <cerastes.h>
#include <stddef.h>

int bump(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && *p < 100)
    CERASTES_ENSURES(r, cerastes_owns(*p) && *p == cerastes_old(*p) + 1 && r == *p)
{
  *p += 1;
  return *p;
}

int peek(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(r, cerastes_reads(*p) && r == *p)
{
  return *p;
}

int owns_and_reads(int c)
    CERASTES_ENSURES(r, r >= 0 || r < 0)
{
  int x = 1;
  int y = 2;
  int *p = &x;
  int *q = c ? p : &y;
  return bump(p) + peek(q);
}
