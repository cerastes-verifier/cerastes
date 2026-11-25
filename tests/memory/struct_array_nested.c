// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 3 and 5 (S47 (e)): the array member of a nested structure member, t.s.a[i], with the chain loc(member(loc(member(t, 0), 0), 0), i)
// Pre-registered from record 0063 item 5 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

struct T {
  struct S s;
  int z;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, r == 6)
{
  struct T t = {{{1, 2, 3, 4}, 5}, 9};
  t.s.a[i] = 6;
  CERASTES_CHECK(t.z == 9 && t.s.n == 5);
  return t.s.a[i];
}
