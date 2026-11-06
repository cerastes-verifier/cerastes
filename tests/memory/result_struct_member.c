// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0052 item 2 (0045 item 1, question 106; the former pin reject_struct_value_live_result.c and reject_owns_result_member_target.c): live(r.q) and owns(*r.q) of a struct result's pointer member, given: the callee keeps its requires atom on *p and gives the place through the member, with r.q == p
// Pre-registered from record 0052 item 2: verifies in both modes
#include <cerastes.h>

struct Q {
  int *q;
};

struct Q give_live(int *p)
    CERASTES_REQUIRES(cerastes_live(p))
    CERASTES_ENSURES(r, cerastes_live(r.q) && r.q == p)
{
  struct Q w = {p};
  return w;
}

struct Q give_owns(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*r.q) && r.q == p)
{
  struct Q w = {p};
  return w;
}

int take_live(int *q)
    CERASTES_REQUIRES(cerastes_live(q))
    CERASTES_ENSURES(r, cerastes_live(q) && r == 0)
{
  (void)q;
  return 0;
}

static int x = 1;

int main(void)
{
  struct Q a = give_live(&x);
  int z = take_live(a.q);
  struct Q b = give_owns(&x);
  *b.q = 3;
  int v = x;
  CERASTES_CHECK(v == 3);
  return z + (v - 3);
}
