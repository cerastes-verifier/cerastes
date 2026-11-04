// expect: error permission @ L10
// mode: c23, c++20
// twin: tests/memory/heap_guard_partner.c
// Pins: 0045 item 7 (audit round 2): the ensures guard is true on a path (*c == 0) where its requires partner took nothing; no body proves the return, so the contract is refused: permission, 'returns `*q` on a path where its `requires` did not take it'
#include <cerastes.h>
#include <stddef.h>

void f(int *c, int *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*c) && cerastes_owns(*p) && (*c == 0 || cerastes_owns(*q)))
    CERASTES_ENSURES(cerastes_reads(*c) && cerastes_owns(*p) && (*c != 0 || cerastes_owns(*q)));  // <- here

void use(int *c, int *p, int *q)
    CERASTES_REQUIRES(cerastes_reads(*c) && cerastes_owns(*p) && cerastes_owns(*q))
{
  f(c, p, q);
}
