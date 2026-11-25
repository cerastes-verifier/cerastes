// expect: error permission @ L18
// mode: c23, c++20
// twin: tests/memory/struct_array_copy_pointers.c
// Pins: 0063 item 13: the destination of a copy is written: *d = *s with only reads(*d) is permission at the copy (the destination's array leaf at `write`, a marked row)
// Pre-registered from record 0063 item 13 (Part E), before any run: the one error (permission at the copy)
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void copy(struct S *d, struct S *s)
    CERASTES_REQUIRES(cerastes_reads(*d) && cerastes_reads(*s))
    CERASTES_ENSURES(cerastes_reads(*d) && cerastes_reads(*s))
{
  *d = *s;  // <- here
}

void self_copy(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[1] == cerastes_old(p->a[1]) && p->n == cerastes_old(p->n))
{
  *p = *p;
}

int use(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t = {{0, 0, 0, 0}, 0};
  copy(&t, &s);
  return t.a[2] * 0 + s.n * 0;
}
