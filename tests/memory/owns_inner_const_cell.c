// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 13 (P70 Q2): int *const *pp: the cell *pp is const but **pp is a plain int the function may write, so owns(**pp) is accepted (framed by reads(*pp))
#include <cerastes.h>
#include <stddef.h>

void set(int *const *pp)
    CERASTES_REQUIRES(cerastes_reads(*pp) && cerastes_owns(**pp))
    CERASTES_ENSURES(cerastes_reads(*pp) && cerastes_owns(**pp) && **pp == 5)
{
  **pp = 5;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int x = 0;
  int *const p = &x;
  set(&p);
  return x;
}
