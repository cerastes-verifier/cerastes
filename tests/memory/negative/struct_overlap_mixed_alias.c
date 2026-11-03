// expect: error permission @ L23
// mode: c23, c++20
// twin: tests/memory/struct_overlap_mixed.c
// Pins: 0043 item 9: the owns atom on &s.x and the reads atom on *s name one cell: the separation fails at the call
#include <cerastes.h>

struct S {
  int x;
  int y;
};

int mixed(struct S *s, int *q)
    CERASTES_REQUIRES(cerastes_reads(*s) && cerastes_owns(*q))
    CERASTES_ENSURES(r, cerastes_reads(*s) && cerastes_owns(*q) && r == s->y)
{
  (void)q;
  return s->y;
}

int use_mixed(void)
{
  struct S s = {3, 4};
  return mixed(&s, &s.x);  // <- here
}
