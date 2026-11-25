// expect: verified
// expect: native-ok
// mode: c23, c++20
// (a comment line keeps the line numbers of the program this test replaces)
// Pins: 0063 items 5 and 6 (flip A of M4.5e1e1, the staging pin struct_array_reject_local_subscript.c retired): a structure local's array member read by index in f (unchanged, line for line), and written and read beside the other member in g (the member's flags follow its elements: an unwritten element stays unwritten)
// Pre-registered from record 0063 item 5 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int f(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
{
  struct S s = {{1, 2, 3, 4}, 5};
  return s.a[i];
}

int g(void)
    CERASTES_ENSURES(r, r == 7)
{
  struct S s;
  s.n = 3;
  s.a[1] = 4;
  CERASTES_CHECK(s.a[1] + s.n == 7);
  s.a[2] = s.a[1] * 2;
  return s.a[1] + s.n;
}
