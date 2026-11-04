// expect: verified
// expect: native-ok
// mode: c23, c++20
// Pins: 0045 item 3 (audit round 2 S1): an ensures value read under a heap guard whose requires atom has the same guard (the guard reads *c, owned by reads in both states): the old(**pp) of the postcondition is taken under the entry guard, framed by the requires atom; written as the auditor's probe implied_old.c
#include <cerastes.h>
#include <stddef.h>

void f(int **pp, int *c)
    CERASTES_REQUIRES(cerastes_reads(*c) && cerastes_owns(*pp) && (*c == 0 || cerastes_owns(**pp)))
    CERASTES_REQUIRES(*c == 0 || **pp < 100)
    CERASTES_ENSURES(cerastes_reads(*c) && cerastes_owns(*pp) && *pp == cerastes_old(*pp) && (*c == 0 || cerastes_owns(**pp)))
    CERASTES_ENSURES(*c == 0 || **pp == cerastes_old(**pp) + 1)
{
  if (*c != 0) {
    **pp = **pp + 1;
  }
}
