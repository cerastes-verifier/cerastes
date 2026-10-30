// expect: error permission @ L11
// expect: error permission @ L12
// mode: c23, c++20
// twin: tests/memory/conditional_ownership_trusted.c
// Pins: 0039 item 7, partners by path (question 72, audit S7): a bodyless contract that takes owns(*p) when c and reads(*p) otherwise but returns reads(*p) when c and owns(*p) otherwise: each ensures atom holds on a path where its requires partner was not taken: permission at each ensures atom (the caller's ensures holds for any state, so it is not what fails)
#include <cerastes.h>
#include <stddef.h>

void lend(int *p, int c)
    CERASTES_REQUIRES(c ? cerastes_owns(*p) : cerastes_reads(*p))
    CERASTES_ENSURES(c == 0 || cerastes_reads(*p))
    CERASTES_ENSURES(c != 0 || cerastes_owns(*p));

void h(int *p)
    CERASTES_REQUIRES(cerastes_owns(*p))
    CERASTES_ENSURES(cerastes_owns(*p))
{
  lend(p, 0);
}
