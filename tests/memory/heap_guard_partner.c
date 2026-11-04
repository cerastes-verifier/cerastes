// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 items 6 and 7 (B1): a bodiless contract whose ensures atom under a heap guard has the guard of its requires partner: it returns *q exactly where the partner took it (the caller makes its contract checked)
#include <cerastes.h>
#include <stddef.h>

void f(int *c, int *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)))
    CERASTES_ENSURES(cerastes_reads(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)));

void use(int *c, int *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*c) && cerastes_owns(*p) && cerastes_owns(*q))
{
  f(c, p, q);
}
