// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 13 (flip A of M4.5e1e1, the staging pin struct_array_reject_copy.c retired): the whole-structure copy `t = s` of a structure with an array member, f as the pin had it (L18), then a copy to an initialized local, an initializer from an object and the independence of the copies: every source element and flag is read, every destination element written (twins negative/struct_array_copy_wrong.c, struct_array_copy_unwritten.c)
// Pre-registered from record 0063 item 13 (Part E), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(void)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t;
  t = s;
  return t.n;
}

int values(void)
    CERASTES_ENSURES(r, r == 8)
{
  struct S s = {{1, 2, 3, 4}, 5};
  struct S t = {{0, 0, 0, 0}, 0};
  t = s;
  CERASTES_CHECK(t.a[2] == 3 && t.n == 5 && s.a[2] == 3);
  struct S u = s;
  CERASTES_CHECK(u.a[3] == 4);
  t.a[0] = 9;
  CERASTES_CHECK(s.a[0] == 1 && t.a[0] == 9 && u.a[0] == 1);
  return t.a[2] + u.n;
}
