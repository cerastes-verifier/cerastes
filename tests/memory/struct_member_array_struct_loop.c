// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0064 item 10 as built in Part C2 (audit round C2, finding 3, a loop over a member array of structures): a member of a structure local or of a held structure place is a range over structures with a literal count once its first slot is bound, so a loop that writes t.v[0].y keeps t.v[1].x by the loop's own invariant clause, and a loop that writes p->v[k].x through a pointer parameter under owns(*p) keeps p->v[1].x when the written value agrees with it (3 + k - 1 is 3 only for k == 1, and p->v[0].x is another slot). The twin negative/struct_member_array_struct_loop_alias.c writes through a pointer into the member array, or breaks the clause
// Registered from audit-C2 probes g03b c and e (measured on snapshot a044aff629b8: verified, the checks and clauses of c and e hold) with the probes' bodies laid out as this file, before its first run on this build; local_loop's ensures and its return are added (r == 3 follows from the clause), unmeasured
#include <cerastes.h>

struct P {
  int x;
  int y;
};

struct T {
  struct P v[2];
  int n;
};

int local_loop(void)
    CERASTES_ENSURES(r, r == 3)
{
  struct T t = {{{1, 2}, {3, 4}}, 5};
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && t.v[1].x == 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    t.v[0].y = i;
    i = i + 1;
  }
  CERASTES_CHECK(t.v[1].x == 3);
  return t.v[1].x;
}

int param_loop(struct T *p, int k)
    CERASTES_REQUIRES(cerastes_owns(*p) && p->v[1].x == 3 && 0 <= k && k < 2)
    CERASTES_ENSURES(r, cerastes_owns(*p) && p->v[1].x == 3 && r == 3)
{
  int i = 0;
  CERASTES_INVARIANT(0 <= i && i <= 3 && p->v[1].x == 3)
  CERASTES_DECREASES(3 - i)
  while (i < 3) {
    p->v[k].x = 3 + k - 1;
    i = i + 1;
  }
  return p->v[1].x;
}
