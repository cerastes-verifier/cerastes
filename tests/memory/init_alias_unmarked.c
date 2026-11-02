// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0042 item 6 (audit S4): the same alias reads with unmarked parameters: any atom of the field frames them (0037 item 7); the marked twins fail permission in the contract
#include <cerastes.h>

static int g = 0;

void req_read(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p)
    CERASTES_REQUIRES(*q > 0)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
  (void)q;
}

void old_read(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p && *p == 0)
    CERASTES_ENSURES(cerastes_owns(*p) && q == p && cerastes_old(*q) == 0)
{
  (void)p;
  (void)q;
}

void dec_read(int *p, int *q)
    CERASTES_REQUIRES(cerastes_owns(*p) && q == p && *p >= 0)
    CERASTES_DECREASES(*q)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
  (void)q;
}

int *pass(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(r, cerastes_owns(*p) && r == p && *r == 1)
{
  *p = 1;
  return p;
}

void glob_read(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g && g == 0)
    CERASTES_ENSURES(cerastes_owns(*p))
{
  (void)p;
}

void glob_old(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p) && p == &g && g == 0)
    CERASTES_ENSURES(cerastes_owns(*p) && cerastes_old(g) == 0)
{
  (void)p;
}

int main(void)
{
  return g;
}
