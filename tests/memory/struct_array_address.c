// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0063 item 5: &s.a[i] is the address of a slot of the member block: stored in a pointer, written through and read back, and aliasing the element named by the index
// Pre-registered from record 0063 item 5 (Part C), before any run: verifies
#include <cerastes.h>
#include <stddef.h>

struct S {
  int a[4];
  int n;
};

int use(int i)
    CERASTES_REQUIRES(0 <= i && i < 4)
    CERASTES_ENSURES(r, r == 8)
{
  struct S s = {{1, 2, 3, 4}, 5};
  int *q = &s.a[2];
  *q = 8;
  int *w = &s.a[i];
  CERASTES_CHECK(*w == s.a[i]);
  CERASTES_CHECK(s.a[2] == 8 && s.n == 5);
  return *q + (*w - *w);
}
