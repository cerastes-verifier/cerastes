// expect: verified
// expect: native-ok
// mode: c23, c++20
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0063 item 13 (flip A of M4.5e1e1, the staging pin struct_array_reject_by_value_param.c retired): a by-value parameter of a structure with an array member (f as the pin had it, L15), one read in a requires and changed in the callee: the callee changes its copy and the caller's object is unchanged (twin negative/struct_array_by_value_wrong.c)
// Pre-registered from record 0063 item 13 (Part E), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(struct S s)
{
  return s.n;
}

int bump(struct S s)
    CERASTES_REQUIRES(s.a[1] == 2)
    CERASTES_ENSURES(r, r == 3)
{
  s.a[1] = s.a[1] + 1;
  return s.a[1];
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int v = bump(s);
  CERASTES_CHECK(v == 3 && s.a[1] == 2);
  return s.n + v * 0;
}
