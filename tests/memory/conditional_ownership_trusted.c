// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0039 item 7, partners by path (question 72, audit S7): an ensures atom whose guard implies its requires partner's guard, bodyless (trusted) and with a body, called with 1, 0 and an unknown c: the caller keeps its cell and its value
#include <cerastes.h>
#include <stddef.h>

void lend(int *p, int c)
    CERASTES_REQUIRES(c ? cerastes_owns(*p) : cerastes_reads(*p))
    CERASTES_ENSURES((c ? cerastes_owns(*p) : cerastes_reads(*p)) && *p == cerastes_old(*p));

void lend_body(int *p, int c)
    CERASTES_REQUIRES(c ? cerastes_owns(*p) : cerastes_reads(*p))
    CERASTES_ENSURES((c ? cerastes_owns(*p) : cerastes_reads(*p)) && *p == cerastes_old(*p))
{
  (void)p;
  (void)c;
}

int use_lend(int c)
    CERASTES_ENSURES(r, r == 7)
{
  int x = 7;
  lend(&x, 1);
  lend(&x, 0);
  lend(&x, c);
  lend_body(&x, 1);
  lend_body(&x, 0);
  lend_body(&x, c);
  return x;
}
