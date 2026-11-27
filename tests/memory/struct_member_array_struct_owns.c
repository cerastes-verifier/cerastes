// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 items 3, 7 and 8 (Part C2): owns(*p) of a structure whose member is an array of structures (a callee that writes p->v[i].x and p->n; an element leaf read with old in a contract) and owns(p->v) alone, the array member as the atom (zero_first writes the first element; the caller's other member t.n is not given away and keeps its value), each called with the address of a local
// Pre-registered from record 0064 items 3, 7 and 8 (Part C2), before any run: verifies [CORRECTED once under question 160 (first run, j3a-t1): set's ensures did not say that v[1].y is kept, so bump's requires p->v[1].y < 100 and the check t.v[1].y == 5 after it failed (precondition @ L46, check @ L47: the design error was the file's, the post havocs the other leaves); the clause `p->v[1].y == cerastes_old(p->v[1].y)` is added to set's ensures, no line moves]
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

void set(struct T *p, int i, int a)
    CERASTES_REQUIRES(cerastes_owns(*p) && 0 <= i && i < 2)
    CERASTES_ENSURES(cerastes_owns(*p) && p->v[i].x == a && p->n == i && p->v[1].y == cerastes_old(p->v[1].y))
{
  p->v[i].x = a;
  p->n = i;
}

void bump(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->v[1].y < 100)
    CERASTES_ENSURES(cerastes_owns(*p) && p->v[1].y == cerastes_old(p->v[1].y) + 1 && p->n == cerastes_old(p->n))
{
  p->v[1].y += 1;
}

void zero_first(struct T *p)
    CERASTES_REQUIRES(cerastes_owns(p->v))
    CERASTES_ENSURES(cerastes_owns(p->v) && p->v[0].x == 0)
{
  p->v[0].x = 0;
}

int use(void)
    CERASTES_ENSURES(r, r == 0)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  set(&t, 1, 9);
  CERASTES_CHECK(t.v[1].x == 9 && t.n == 1);
  bump(&t);
  CERASTES_CHECK(t.v[1].y == 5);
  zero_first(&t);
  CERASTES_CHECK(t.v[0].x == 0 && t.n == 1);
  return t.v[0].x;
}
