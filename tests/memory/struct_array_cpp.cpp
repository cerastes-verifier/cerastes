// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0063 items 6 and 7: the C++ spellings cerastes::owns(*p), cerastes::reads(p->a) and cerastes::old(p->a[i]) name array members like the C ones; S s{{...}, n} and a designated list
// Pre-registered from record 0063 item 7 (Part C), before any run: verifies
#include <cerastes.h>

struct S {
  int a[4];
  int n;
};

void bump(S* p, int i)
    CERASTES_REQUIRES(cerastes::owns(*p) && 0 <= i && i < 4 && p->a[i] < 100)
    CERASTES_ENSURES(cerastes::owns(*p) && p->a[i] == cerastes::old(p->a[i]) + 1)
{
  p->a[i] += 1;
}

int first(S* p)
    CERASTES_REQUIRES(cerastes::reads(p->a))
    CERASTES_ENSURES(r, cerastes::reads(p->a) && r == p->a[0])
{
  return p->a[0];
}

int use()
    CERASTES_ENSURES(r, r == 2)
{
  S s{{1, 2, 3, 4}, 5};
  S t = {.a = {1, 2}, .n = 3};
  bump(&s, 0);
  CERASTES_CHECK(s.a[0] == 2 && t.a[3] == 0);
  return first(&s) + t.n - 3;
}
