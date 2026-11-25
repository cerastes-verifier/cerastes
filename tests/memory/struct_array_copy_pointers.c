// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 13: `*d = *s` through pointers (d owned, s read: the array leaf's source amount is the read amount, the copy's rd), the guard d != s written for pointer places, the exact self-copy `*p = *p` (guarded out), the source unchanged (twins negative/struct_array_copy_dest_reads.c, struct_array_copy_self_wrong.c)
// Pre-registered from record 0063 item 13 (Part E), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

void copy(struct S *d, struct S *s)
    CERASTES_REQUIRES(cerastes_owns(*d) && cerastes_reads(*s))
    CERASTES_ENSURES(cerastes_owns(*d) && cerastes_reads(*s) && d->a[2] == s->a[2] && d->n == s->n)
{
  *d = *s;
}

void self_copy(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[2] == cerastes_old(p->a[2]) && p->n == cerastes_old(p->n))
{
  *p = *p;
}

int use(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t = {{0, 0, 0, 0}, 0};
  copy(&t, &s);
  CERASTES_CHECK(t.a[2] == 3 && t.n == 5 && s.a[2] == 3);
  self_copy(&t);
  CERASTES_CHECK(t.a[2] == 3 && t.n == 5);
  return t.a[2];
}
