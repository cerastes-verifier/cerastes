// expect: verified
// expect: native-ok
// mode: c++20
// Pins: 0043 item 9: the C++ spellings cerastes::owns(*p), cerastes::reads(p->x) and cerastes::old(p->x) name struct places like the C ones
#include <cerastes.h>

struct S {
  int x;
  int y;
};

void bump(S* p)
    CERASTES_REQUIRES(cerastes::owns(*p) && p->x < 100)
    CERASTES_ENSURES(cerastes::owns(*p) && p->x == cerastes::old(p->x) + 1 && p->y == cerastes::old(p->y))
{
  p->x += 1;
}

int use_bump()
    CERASTES_ENSURES(r, r == 5)
{
  S s{4, 9};
  bump(&s);
  CERASTES_CHECK(s.y == 9);
  return s.x;
}

int peek(S* p)
    CERASTES_REQUIRES(cerastes::reads(p->x))
    CERASTES_ENSURES(r, cerastes::reads(p->x) && r == p->x)
{
  return p->x;
}
