// expect: verified
// expect: native-ok
// c-only: compound literal objects (&(T){...}) do not exist in C++ (decision 0044 item 7)
// Pins: 0063 item 13: a compound literal of a structure with an array member as a by-value argument (an Aggregate with one value per element) and under & (a hidden local of its block, owned for the call) (twins negative/struct_array_literal_wrong.c, struct_array_literal_check_wrong.c)
// Pre-registered from record 0063 item 13 (Part E), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int get(struct S s)
    CERASTES_REQUIRES(s.a[2] == 3)
    CERASTES_ENSURES(r, r == s.a[2])
{
  return s.a[2];
}

void set(struct S *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p) && p->a[1] == 9)
{
  p->a[1] = 9;
}

int by_value(void)
    CERASTES_ENSURES(r, r == 3)
{
  return get((struct S){{1, 2, 3, 4}, 5});
}

int under_address(void)
    CERASTES_ENSURES(r, r == 9)
{
  struct S *p = &(struct S){{1, 2, 3, 4}, 5};
  set(p);

  return p->a[1];
}
