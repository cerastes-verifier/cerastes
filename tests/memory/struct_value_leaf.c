// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 3, 9: owns(*s.q), reads(*s.q) and live(s.q) of a by-value parameter's pointer member (the leaf formal is a fixed root), *s.q, and return s.q with live(s.q)
#include <cerastes.h>
#include <stddef.h>
struct P {
  int *q;
};

int deref(struct P s)
    CERASTES_REQUIRES(cerastes_owns(*s.q) && cerastes_live(s.q))
    CERASTES_ENSURES(r, cerastes_owns(*s.q) && cerastes_live(s.q) && *s.q == cerastes_old(*s.q) && r == *s.q)
{
  return *s.q;
}

int rd(struct P s)
    CERASTES_REQUIRES(cerastes_reads(*s.q) && *s.q == 4)
    CERASTES_ENSURES(r, cerastes_reads(*s.q) && r == 4)
{
  return *s.q;
}

int *ident(struct P s)
    CERASTES_REQUIRES(cerastes_live(s.q))
    CERASTES_ENSURES(r, cerastes_live(s.q) && r == s.q)
{
  return s.q;
}

int use(void)
    CERASTES_ENSURES(r, r == 5)
{
  int v = 5;
  struct P s = {&v};
  return deref(s);
}
