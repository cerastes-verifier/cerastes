// expect: error permission @ L32
// mode: c23, c++20
// twin: tests/memory/range_struct_nested_struct.c
// Pins: 0064 item 7: the same call with &v[0].t, inside the range [0, 1): the struct T atom meets the range through t.a and t.b and the pairing row fails at the call (permission, once)
// Pre-registered from record 0064 items 7, 8 and 10 (Part C1), before any run: permission at the call
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
  nested_t(v, 1, &v[0].t);  // <- here
  return v[1].t.a;
}
