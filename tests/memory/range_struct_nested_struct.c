// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 7 (the alias through a nested structure: a struct T atom on &v[1].t against owns_range over struct S, paired leaf by leaf through t.a and t.b): the caller passes the nested structure of element 1, outside the range [0, 1)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: verifies (timed at the first run; the plan's rule 3)
#include <cerastes.h>
#include <stddef.h>

struct T {
  int a;
  int b;
};

struct S {
  int x;
  struct T t;
};

void nested_t(struct S *p, int n, struct T *q)
    CERASTES_REQUIRES(cerastes_owns_range(p, n) && cerastes_owns(*q))
    CERASTES_ENSURES(cerastes_owns_range(p, n) && cerastes_owns(*q) && q->a == 5)
{
  (void)p;
  (void)n;
  q->a = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S v[2] = {{1, {2, 3}}, {4, {5, 6}}};
  nested_t(v, 1, &v[1].t);
  return v[1].t.a;
}
