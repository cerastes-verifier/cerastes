// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0062 items 3 and 7: a pure function over a cell atom (reads(*p)) applied in a check on a local whose address the code takes (0038 item 6: `&x` in an annotation is refused where the code never takes it), through a pointer variable
// Pre-registered from record 0062 (Part B), before the code: verifies. Rewritten before any run (edg-backend's half): the first version took `&x` in the check only. Edited before any run (audit round 2, R2-S3): the check observes the value, so a wrong slot or swapped arguments fails; its off-by-one twin is the negative of the same name with _value_off_by_one
#include <cerastes.h>

CERASTES_PURE int get(const int *p)
    CERASTES_REQUIRES(cerastes_reads(*p))
{
  return *p;
}

int use(void)
{
  int x = 5;
  int *q = &x;
  CERASTES_CHECK(get(q) == 5);
  return *q;
}
