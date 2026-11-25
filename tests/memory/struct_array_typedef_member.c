// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 items 2, 5 and 14 (left to Part C by Part D): a member declared through a typedef of an array type, its element access, and sizeof(s.a), whose value is the front end's with the member's bound checked as written
// Pre-registered from record 0063 item 14 (Part C), before any run: verifies
#include <cerastes.h>

typedef int T3[3];

struct S {
  T3 a;
  int n;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 3)
    CERASTES_ENSURES(r, r == 12)
{
  struct S s = {{1, 2, 3}, 4};
  s.a[i] = 8;
  CERASTES_CHECK(sizeof(s.a) == 3 * sizeof(int));
  CERASTES_CHECK(sizeof(s.a) / sizeof(s.a[0]) == 3);
  return s.n + s.a[i];
}
