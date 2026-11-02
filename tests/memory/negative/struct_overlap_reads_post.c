// expect: error postcondition @ L23
// mode: c23, c++20
// twin: tests/memory/struct_overlap_reads.c
// Pins: 0043 item 9: sum(&s, &s.x) counts s.x twice and gives 10, not 11
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int sum(struct S *s, int *q)
    CERASTES_REQUIRES(cerastes_reads(*s) && cerastes_reads(*q) && 0 <= s->x && s->x <= 1000 && 0 <= s->y && s->y <= 1000 && 0 <= *q && *q <= 1000)
    CERASTES_ENSURES(r, cerastes_reads(*s) && cerastes_reads(*q) && r == s->x + s->y + *q)
{
  return s->x + s->y + *q;
}

int use_sum(void)
    CERASTES_ENSURES(r, r == 11)
{
  struct S s = {3, 4};
  return sum(&s, &s.x);  // <- here
}
