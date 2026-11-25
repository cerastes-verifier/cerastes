// expect: error permission @ L22
// mode: c23, c++20
// twin: tests/memory/struct_array_loops.c
// Pins: 0063 items 7 and 10: a write to p->a[i] in a loop of a function that holds *p only through reads: no write permission, permission at the store
// Pre-registered from record 0063 item 10 (Part C), before any run: the one error
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void clear(struct S *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
    CERASTES_ENSURES(cerastes_reads(*p))
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 4)
  CERASTES_DECREASES(4 - i)
  while (i < 4) {
    p->a[i] = 0;  // <- here
    i = i + 1;
  }
}
