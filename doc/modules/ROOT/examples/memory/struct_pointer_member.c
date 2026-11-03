// expect: verified
// expect: native-ok
// mode: c23, c++20
// A contract that follows a pointer member: owns(p->q->x) names a member of the object p->q points to, owns(*s.q) the object behind a by-value parameter's pointer member.
// tag::source[]
#include <cerastes.h>

struct L {
  struct L *q;
  int x;
};

struct P {
  int *q;
};

void inc(struct L *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && cerastes_owns(p->q->x) && p->q->x < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->q == cerastes_old(p->q) && cerastes_owns(p->q->x) && p->q->x == cerastes_old(p->q->x) + 1)
{
  p->q->x += 1;
}

int deref(struct P s)
    CERASTES_REQUIRES(cerastes_owns(*s.q))
    CERASTES_ENSURES(r, cerastes_owns(*s.q) && *s.q == cerastes_old(*s.q) && r == *s.q)
{
  return *s.q;
}

int use(void)
    CERASTES_ENSURES(r, r == 4)
{
  struct L t = {0, 3};
  struct L s = {&t, 0};
  inc(&s);
  CERASTES_CHECK(t.x == 4);
  int v = 4;
  struct P w = {&v};
  return deref(w);
}
// end::source[]
