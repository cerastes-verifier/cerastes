// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 3, the bullet "A length quantifier under another quantifier" (audit round C2, finding 1), the array-of-pointers family: a structure whose member is an array of pointers to a structure with an array member of structures, under owns(*p) (the audit's k05: set_m writes and reads the int member), and a local array of such pointers beside a loop (k03: the loop zeroes the pointers and the structure the first pointed to keeps its value). The twin negative/struct_pointer_array_member_wrong.c claims a wrong value in each
// Registered from audit-C2 probes k05 and k03 (exit 3 on snapshot a044aff629b8) and il-analyst-report-11 (c) (w8p: `p->m = 1; check(p->m == 1)` verified; w4p: `check(t.n == 5)` after the loop verified, on the fixed build); the returns are added (the native compile wants t read), unmeasured
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

struct H2 {
  struct T *ptrs[2];
  int m;
};

int set_m(struct H2 *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == 0)
{
  p->m = 1;
  CERASTES_CHECK(p->m == 1);
  return 0;
}

int zero_all(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  [[maybe_unused]] struct T *a[2] = {&t, 0};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 2)
  CERASTES_DECREASES(2 - i)
  while (i < 2) {
    a[i] = 0;
    i = i + 1;
  }
  CERASTES_CHECK(t.n == 5);
  return t.n;
}
