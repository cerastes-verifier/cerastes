// expect: rejected cerastes-unsupported_annotation @ L8
// mode: c23, c++20
// twin: tests/memory/not_invalidating_int_pointer.c
// Pins: 0058 item 2 (as revised): a partnerless ensures atom rooted at the marked parameter (owns(*p) in the ensures, none in the requires): the contract gives a cell, which the marker, that returns what it takes and gives nothing new through p, forbids. The function has a body so that 0052 item 2a (which rejects a bodiless contract's partnerless atom at the ensures clause) does not fire first; a rejection is reported before verification, so the body's own failure (emul/k3.c: permission) does not show
// Pre-registered from record 0058 item 2 (as revised): rejected unsupported_annotation at the parameter; question 160 if the marked form with a body reports another line
#include <cerastes.h>

void give(int *p CERASTES_NOT_INVALIDATING)  // <- here
    CERASTES_ENSURES(cerastes_owns(*p))
{
  *p = 0;
}

int use(void)
{
  int x = 1;
  give(&x);
  return 0;
}
